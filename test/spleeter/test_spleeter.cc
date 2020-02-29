#include <gtest/gtest.h>

#include <cassert>
#include "wave/file.h"
#include "spleeter/spleeter.h"

#include "spleeter/model.h"
#include "spleeter/registry.h"

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

/// Copy an fft frame into to tensorflow tensor
/// \tparam The type of data the tensor holds
/// \param tensor
/// \param frame_index the destination frame index in the tensor
/// \param data the fft frame data
template <typename T>
void SetTensorFrame(tensorflow::Tensor* tensor, uint32_t frame_index, std::vector<T*> data) {
  auto bin_size = tensor->shape().dim_size(1);
  
  auto eigen_input = tensor->tensor<T, 3>();
  for (auto bin_index = 0; bin_index < bin_size; bin_index++) {
    for (auto channel_index = 0; channel_index < data.size(); channel_index++) {
      eigen_input(frame_index, bin_index, channel_index) = data[channel_index][bin_index];
    }
  }
}

/// Copy a frame into a vector of data pointers
/// \tparam The type of data the tensor holds
/// \param data the output fft frame data
/// \param frame_index the source frame index in the tensor
/// \param tensor
template <typename T>
void GetTensorFrame(std::vector<T*>* data, uint32_t frame_index, const tensorflow::Tensor& tensor) {
  auto bin_size = tensor.shape().dim_size(1);
  
  auto eigen_input = tensor.tensor<T, 3>();
  for (auto bin_index = 0; bin_index < bin_size; bin_index++) {
    for (auto channel_index = 0; channel_index < data->size(); channel_index++) {
      (*data)[channel_index][bin_index] = eigen_input(frame_index, bin_index, channel_index);
    }
  }
}

/// Move a tensor frame
/// \param tensor
/// \param source_index the source frame index
/// \param destination_index the destination frame index
void MoveTensorFrame(tensorflow::Tensor& tensor,
                     uint16_t source_index,
                     uint16_t destination_index) {
  auto frame_size = tensor.shape().dim_size(1);
  auto channel_count = tensor.shape().dim_size(2);

  auto eigen_input = tensor.tensor<std::complex<float>, 3>();
  for (auto bin_index = 0; bin_index < frame_size; bin_index++) {
    for (auto channel_index = 0; channel_index < channel_count; channel_index++) {
      eigen_input(destination_index, bin_index, channel_index) = \
        eigen_input(source_index, bin_index, channel_index);
    }
  }
}

TEST(Spleeter, Spectrogram) {
  Eigen::Tensor<float, 3> tensor(64, 1, 2);
  std::error_code err;

  // read a file
  wave::File file;
  std::string test_file(TEST_FILE);
//  std::string test_file("/Users/gvne/Desktop/snipet.wav");
  file.Open(test_file, wave::kIn);
  auto data = file.Read(err);
  ASSERT_FALSE(err);
  
  // stft parameters
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

  const auto separation_type = spleeter::TwoStems;

  spleeter::Initialize(std::string(SPLEETER_MODELS), separation_type, err);
  ASSERT_FALSE(err);

  // Find the right model
  auto bundle = spleeter::Registry::instance().Get(separation_type);
  ASSERT_TRUE(bundle);

  // ----------------------------------------------------------------------
  // PARAMETERS
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
  // ----------------------------------------------------------------------
  const auto FrameLatency = T - ((T - FrameLength) / 2);
  
  // Initialize the buffers
  tensorflow::TensorShape shape {T + OverlapLength, half_frame_length, 2};
  tensorflow::Tensor network_input(tensorflow::DT_COMPLEX64, shape);
  tensorflow::Tensor previous_network_input(tensorflow::DT_COMPLEX64, shape);
  tensorflow::Tensor network_output(tensorflow::DT_FLOAT, shape);
  std::vector<tensorflow::Tensor> network_result;
  
  // -- Organize a mask data buffer for a single frame
  std::vector<std::vector<float>> mask_vec_data;
  std::vector<std::vector<float>> previous_mask_vec_data;
  std::vector<float*> mask_data;
  std::vector<float*> previous_mask_data;
  for (auto c = 0; c < filter.channel_count(); c++) {
    mask_vec_data.push_back(std::vector<float>(half_frame_length, 0));
    mask_data.push_back(mask_vec_data[c].data());
    previous_mask_vec_data.push_back(std::vector<float>(half_frame_length, 0));
    previous_mask_data.push_back(previous_mask_vec_data[c].data());
  }
  // --
  
  uint32_t frame_index = 0;
  filter.execute = [bundle,
                    &frame_index,
                    &network_input,
                    &network_result,
                    &network_output,
                    &mask_data,
                    &previous_mask_data,
                    &previous_network_input]
    (std::vector<std::complex<float>*> data, uint32_t size) {
      // Set the frame into the input
      auto network_input_frame_index = frame_index + (T - FrameLatency);
      SetTensorFrame(&network_input, network_input_frame_index, data);
      
      // Compute the output
      GetTensorFrame(&data, network_input_frame_index, previous_network_input);
      GetTensorFrame(&mask_data, network_input_frame_index, network_output);
      for (auto c = 0; c < data.size(); c++) {
        Eigen::Map<Eigen::VectorXcf>(data[c], size).array() *= Eigen::Map<Eigen::VectorXf>(mask_data[c], size).array();
      }
      
      if (frame_index == FrameLength - 1) {
        auto status = bundle->session->Run(
            {{"Placeholder", network_input}}, GetOutputNames(separation_type), {}, &network_result);
        ASSERT_TRUE(status.ok());
        // Overlap --> Update the result by adding the previous output frame and devide by 2
        // TODO: use a cross fade instead of a mean to handle overlap
        for (auto overlap_frame_index = 0; overlap_frame_index < OverlapLength; overlap_frame_index++) {
          auto network_output_index = overlap_frame_index + (T - FrameLatency);
          auto previous_network_output_index = network_output_index + FrameLength;

          GetTensorFrame(&previous_mask_data, previous_network_output_index, network_output);
          GetTensorFrame(&mask_data, network_output_index, network_result[0]);
          for (auto c = 0; c < data.size(); c++) {
            Eigen::Map<Eigen::VectorXf> mask_frame(mask_data[c], size);
            mask_frame += Eigen::Map<Eigen::VectorXf>(previous_mask_data[c], size);
            mask_frame /= 2;
          }
          SetTensorFrame(&(network_result[0]), network_output_index, mask_data);
        }
        // TODO: this is most likely to allocate memory. Redevelop the deep copy to do a simple memcpy
        network_output = tensorflow::tensor::DeepCopy(network_result[0]);
        previous_network_input = tensorflow::tensor::DeepCopy(network_input);
        
        // shift the input data of FrameLength
        for (auto source_index = T - FrameLatency; source_index < T; source_index++) {
          auto destination_index = source_index - FrameLength;
          if (destination_index < 0) {
            continue;
          }
          MoveTensorFrame(network_input, source_index, destination_index);
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
