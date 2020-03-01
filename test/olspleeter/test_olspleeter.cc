#include <gtest/gtest.h>

#include <cassert>
#include "wave/file.h"
#include "spleeter/spleeter.h"

#include "spleeter/model.h"
#include "spleeter/registry.h"
#include "tensor/copy.h"

#include "rtff/filter.h"
#include "tensorflow/cc/framework/ops.h"
#include "tensorflow/core/framework/tensor_util.h"

void Write(const spleeter::Waveform& data, const std::string& name) {
  std::vector<float> vec_data(data.size());
  std::copy(data.data(), data.data() + data.size(), vec_data.data());
  wave::File file;
  file.Open(std::string(OUTPUT_DIR) + "/" + name + ".wav", wave::kOut);
  file.set_sample_rate(44100);
  file.set_channel_number(2);
  file.Write(vec_data);
}

TEST(OLSpleeter, Spectrogram) {
  std::error_code err;

  // read a file
  wave::File file;
//  std::string test_file(TEST_FILE);
  std::string test_file("/Users/gvne/Desktop/snipet.wav");
  file.Open(test_file, wave::kIn);
  auto data = file.Read(err);
  ASSERT_FALSE(err);
  
  // stft parameters Forced by spleeter.
  // TODO: Read it from json file ?
  const auto channel_number = 2;
  const auto frame_length = 4096;
  const auto half_frame_length = frame_length / 2 + 1;
  const auto frame_step = 1024;

  // initialize the rtff filter
  rtff::Filter filter;
  const auto block_size = 2048;
  filter.Init(channel_number, frame_length, frame_length - frame_step, rtff::fft_window::Type::Hann, err);
  ASSERT_FALSE(err);
  filter.set_block_size(block_size);
  rtff::AudioBuffer buffer(block_size, channel_number);

  // ------
  // FILTER PARAMETERS
  const auto separation_type = spleeter::FiveStems;
  std::vector<float> volumes = {0.0, 0.0, 0.0, 1.0, 0.0};  // For each stem we set a volume between 0 & 1
  const auto stem_count = volumes.size();
  // ------

  spleeter::Initialize(std::string(SPLEETER_MODELS), separation_type, err);
  ASSERT_FALSE(err);
  auto bundle = spleeter::Registry::instance().Get(separation_type);
  ASSERT_TRUE(bundle);

  // ----------------------------------------------------------------------
  // ALGORITHM PARAMETERS
  // the input size. reducing that value will reduce the latency but also
  // reduces the temporal information and will lower the quality
  const auto T = SPLEETER_INPUT_FRAME_COUNT;
  // number of frames to process at a time. Always <= T
  const auto FrameLength = T;
  // NOTE:
  // Every time we run a process, we will do it on SPLEETER_INPUT_FRAME_COUNT.
  // However, if we decide to process smaller frames, it will get more CPU
  // intensive as we will process more often but it will reduce the latency.
  // Latency is T - (T - FrameLength) / 2
  // If FrameLength = 1, latency is ~T/2 as we always need to process the
  // center of the matrix. It is the one that beneficit the most of temporal
  // information.
  //
  // We also cross fade between processes to reduce the inconsistency between
  // independent processes
  const auto OverlapLength = 0;
  // The sum of each stem may not be conservative. We can force conservativity
  // at runtime though (just need to devide each mask per the mask sum)
  const bool ForceConservativity = false;
  // ----------------------------------------------------------------------
  const auto FrameLatency = T - ((T - FrameLength) / 2);
  
  // Initialize the buffers
  // -- inputs
  tensorflow::TensorShape shape {T + OverlapLength, half_frame_length, 2};
  tensorflow::Tensor network_input(tensorflow::DT_COMPLEX64, shape);
  tensorflow::Tensor previous_network_input(tensorflow::DT_COMPLEX64, shape);
  
  // -- outputs
  std::vector<tensorflow::Tensor> network_result;
  std::vector<tensorflow::Tensor> previous_network_result;
  for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
    previous_network_result.push_back(tensorflow::Tensor(tensorflow::DT_FLOAT, shape));
  }
  
  // -- We also need pre-allocated data to retreive a single frame
  // => For overlap
  std::vector<std::vector<float>> mask_vec_data;
  std::vector<std::vector<float>> previous_mask_vec_data;
  std::vector<std::vector<float>> mask_sum_vec_data;
  std::vector<float*> mask_data;
  std::vector<float*> previous_mask_data;
  std::vector<float*> mask_sum_data;
  for (auto c = 0; c < filter.channel_count(); c++) {
    mask_vec_data.emplace_back(std::vector<float>(half_frame_length, 0));
    mask_data.push_back(mask_vec_data[c].data());
    previous_mask_vec_data.emplace_back(std::vector<float>(half_frame_length, 0));
    previous_mask_data.push_back(previous_mask_vec_data[c].data());
    mask_sum_vec_data.emplace_back(std::vector<float>(half_frame_length, 0));
    mask_sum_data.push_back(mask_sum_vec_data[c].data());
  }
  // => For masking / volumes
  std::vector<std::vector<std::vector<float>>> masks_vec_data;
  std::vector<std::vector<float*>> masks_data;
  for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
    std::vector<std::vector<float>> single_mask_vec_data;
    std::vector<float*> single_mask_data;
    for (auto c = 0; c < filter.channel_count(); c++) {
      single_mask_vec_data.emplace_back(std::vector<float>(half_frame_length, 0));
      single_mask_data.push_back(single_mask_vec_data[c].data());
    }
    masks_vec_data.emplace_back(std::move(single_mask_vec_data));
    masks_data.emplace_back(std::move(single_mask_data));
  }
  // --
  
  uint32_t frame_index = 0;
  filter.execute = [bundle,
                    volumes,
                    &frame_index,
                    &network_input,
                    &network_result,
                    &previous_network_result,
                    &mask_data,
                    &previous_mask_data,
                    &masks_data,
                    &mask_sum_data,
                    &previous_network_input]
    (std::vector<std::complex<float>*> data, uint32_t size) {
      // --------------------------------
      // Set the frame into the input
      const auto stem_count = previous_network_result.size();
      auto network_input_frame_index = frame_index + (T - FrameLatency);
      tensor::SetFrame(&network_input, network_input_frame_index, data);
      // --------------------------------
      
      // --------------------------------
      // Compute the output
      // -- get the right frame
      tensor::GetFrame(&data, network_input_frame_index, previous_network_input);
      // -- Get each stem mask data
      for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
        tensor::GetFrame(&(masks_data[stem_index]), network_input_frame_index, previous_network_result[stem_index]);
      }
      // -- Apply a mask that is the sum of each masks * volume
      for (auto channel_index = 0; channel_index < data.size(); channel_index++) {
        // force conservativity
        // TODO: this does not work and I don't get why... FIXME !!
        if (ForceConservativity) {
          // -- compute the mask sum (to make it conservative if asked)
          Eigen::Map<Eigen::VectorXf> mask_sum(mask_sum_data[channel_index], size);
          mask_sum.array() *= 0.0;
          for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
            mask_sum += Eigen::Map<Eigen::VectorXf>(masks_data[stem_index][channel_index], size);
          }
          // devide each mask by the mask sum
          for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
            Eigen::Map<Eigen::VectorXf>(masks_data[stem_index][channel_index], size).array() /= mask_sum.array();
          }
        }

        // Apply the volumes
        Eigen::Map<Eigen::VectorXf> result_mask(mask_data[channel_index], size);
        result_mask.array() *= 0.0;
        for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
          Eigen::Map<Eigen::VectorXf> stem_mask(masks_data[stem_index][channel_index], size);
          result_mask.array() += stem_mask.array() * volumes[stem_index];
        }
        
        // Compute the result
        Eigen::Map<Eigen::VectorXcf> fft_frame(data[channel_index], size);
        fft_frame.array() *= result_mask.array();
      }
      // --------------------------------
      
      
      if (frame_index == FrameLength - 1) {
        // --------------------------------
        // Run the extraction !
        auto status = bundle->session->Run(
            {{"Placeholder", network_input}}, GetOutputNames(separation_type), {}, &network_result);
        ASSERT_TRUE(status.ok());
        // Overlap --> Update the result by adding the previous output frame and devide by 2
        // TODO: use a cross fade instead of a mean to handle overlap
        for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
          for (auto overlap_frame_index = 0; overlap_frame_index < OverlapLength; overlap_frame_index++) {
            auto network_output_index = overlap_frame_index + (T - FrameLatency);
            auto previous_network_output_index = network_output_index + FrameLength;

            tensor::GetFrame(&previous_mask_data, previous_network_output_index, previous_network_result[stem_index]);
            tensor::GetFrame(&mask_data, network_output_index, network_result[stem_index]);
            for (auto c = 0; c < data.size(); c++) {
              Eigen::Map<Eigen::VectorXf> mask_frame(mask_data[c], size);
              mask_frame += Eigen::Map<Eigen::VectorXf>(previous_mask_data[c], size);
              mask_frame /= 2;
            }
            tensor::SetFrame(&(network_result[stem_index]), network_output_index, mask_data);
          }
        }
        // TODO: this is most likely to allocate memory. Redevelop the deep copy to do a simple memcpy
        for (auto stem_index = 0; stem_index < stem_count; stem_index++) {
          previous_network_result[stem_index] = tensorflow::tensor::DeepCopy(network_result[stem_index]);
        }
        previous_network_input = tensorflow::tensor::DeepCopy(network_input);
        // --------------------------------
        
        // shift the input data of FrameLength
        for (auto source_index = T - FrameLatency; source_index < T; source_index++) {
          auto destination_index = source_index - FrameLength;
          if (destination_index < 0) {
            continue;
          }
          tensor::MoveFrame(network_input, source_index, destination_index);
        }
        frame_index = 0;
      } else {
        frame_index += 1;
      }
  };

  // Run each frames
  auto multichannel_buffer_size = block_size * channel_number;
  for (uint32_t sample_idx = 0;
     sample_idx < data.size() - multichannel_buffer_size;
     sample_idx += multichannel_buffer_size) {
    
    float* sample_ptr = data.data() + sample_idx;
    buffer.fromInterleaved(sample_ptr);
    filter.ProcessBlock(&buffer);
    buffer.toInterleaved(sample_ptr);
  }

  wave::File result_file;
  result_file.Open(std::string(OUTPUT_DIR) + "/result.wav", wave::kOut);
  result_file.set_sample_rate(44100);
  result_file.set_channel_number(2);
  result_file.Write(data);
}

// TEST(Spleeter, TwoStems) {
//  // Read wav file
//  wave::File file;
//  file.Open(std::string(TEST_FILE), wave::kIn);
//  std::error_code err;
//  auto data = file.Read(err);
//  ASSERT_FALSE(err);
//  auto source = Eigen::Map<spleeter::Waveform>(data.data(), 2, data.size() /
//  2);
//
//  // ------------------------------
//  // Spleeter !
//  spleeter::Initialize(
//      std::string(SPLEETER_MODELS),
//      {spleeter::TwoStems, spleeter::FourStems, spleeter::FiveStems}, err);
//  ASSERT_FALSE(err);
//  {
//    spleeter::Waveform vocals, accompaniment;
//    spleeter::Split(source, &vocals, &accompaniment, err);
//    ASSERT_FALSE(err);
//    Write(vocals, "vocals-2stems");
//    Write(accompaniment, "accompaniment-2stems");
//  }
//  {
//    spleeter::Waveform vocals, drums, bass, other;
//    spleeter::Split(source, &vocals, &drums, &bass, &other, err);
//    ASSERT_FALSE(err);
//    Write(vocals, "vocals-4stems");
//    Write(drums, "drums-4stems");
//    Write(bass, "bass-4stems");
//    Write(other, "other-4stems");
//  }
//  {
//    spleeter::Waveform vocals, drums, bass, piano, other;
//    spleeter::Split(source, &vocals, &drums, &bass, &piano, &other, err);
//    ASSERT_FALSE(err);
//    Write(vocals, "vocals-5stems");
//    Write(drums, "drums-5stems");
//    Write(bass, "bass-5stems");
//    Write(piano, "piano-5stems");
//    Write(other, "other-5stems");
//  }
//}
