#include <gtest/gtest.h>

#include <cassert>
#include "wave/file.h"
#include "spleeter/spleeter.h"

#include "spleeter/model.h"
#include "spleeter/registry.h"

#include "rtff/filter.h"
#include "tensorflow/cc/framework/ops.h"

void Write(const spleeter::Waveform& data, const std::string& name) {
  std::vector<float> vec_data(data.size());
  std::copy(data.data(), data.data() + data.size(), vec_data.data());
  wave::File file;
  file.Open(std::string(OUTPUT_DIR) + "/" + name + ".wav", wave::kOut);
  file.set_sample_rate(44100);
  file.set_channel_number(2);
  file.Write(vec_data);
}

// TODO: find a better way
void SetTensorFrame(std::vector<std::complex<float>*> data,
                    uint32_t size,
                    tensorflow::Tensor& tensor,
                    uint16_t frame_index) {
  auto eigen_input = tensor.tensor<std::complex<float>, 3>();
  for (auto bin_index = 0; bin_index < size; bin_index++) {
    for (auto channel_index = 0; channel_index < data.size(); channel_index++) {
      eigen_input(frame_index, bin_index, channel_index) = data[channel_index][bin_index];
    }
  }
}

void MoveTensorFrame(tensorflow::Tensor& tensor,
                     uint16_t source_index,
                     uint16_t destination_index,
                     uint32_t frame_size,
                     uint8_t channel_count) {
  auto eigen_input = tensor.tensor<std::complex<float>, 3>();
  for (auto bin_index = 0; bin_index < frame_size; bin_index++) {
    for (auto channel_index = 0; channel_index < channel_count; channel_index++) {
      eigen_input(destination_index, bin_index, channel_index) = \
        eigen_input(source_index, bin_index, channel_index);
    }
  }
}

template <typename T>
void CopyTensorBlock(const tensorflow::Tensor& tensor,
                     uint16_t first_frame,
                     uint16_t frame_count,
                     std::vector<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>>* output) {
  auto frame_size = (*output)[0].rows();
  auto eigen_tensor = tensor.tensor<T, 3>();
  
  for (auto frame_index = first_frame; frame_index < first_frame + frame_count; frame_index++) {
    for (auto bin_index = 0; bin_index < frame_size; bin_index++) {
      for (auto channel_index = 0; channel_index < output->size(); channel_index++) {
        (*output)[channel_index](bin_index, frame_index - first_frame) = \
          eigen_tensor(frame_index, bin_index, channel_index);
      }
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

  // 60sec 44.1KHz
//  const auto sample_count = 44100 * 60;
  const auto channel_number = 2;
  // stft parameters
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

//  const auto frame_count = sample_count / frame_step;
//  Eigen::MatrixXcf left = Eigen::MatrixXcf::Random(half_frame_length, frame_count);
//  Eigen::MatrixXcf right = Eigen::MatrixXcf::Random(half_frame_length, frame_count);

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
  const auto FrameLatency = T - (T - FrameLength) / 2;
  const auto StepLength = FrameLength - OverlapLength;
  
  

  // Initialize the input
  std::vector<tensorflow::Tensor> output;
  tensorflow::Tensor input(
      tensorflow::DT_COMPLEX64,
      tensorflow::TensorShape({T + OverlapLength, half_frame_length, 2}));
  
  // We need to keep the original input and output for future process
  std::vector<Eigen::MatrixXcf> input_frames = {
    Eigen::MatrixXcf::Zero(half_frame_length, FrameLength),
    Eigen::MatrixXcf::Zero(half_frame_length, FrameLength)
  };
  std::vector<Eigen::MatrixXf> output_frames = {
    Eigen::MatrixXf::Zero(half_frame_length, FrameLength),
    Eigen::MatrixXf::Zero(half_frame_length, FrameLength)
  };

  uint16_t frame_index = T - FrameLatency;
  uint16_t input_frame_index = 0;
  filter.execute = [T, FrameLatency, StepLength, OverlapLength, bundle, &input, &output, &frame_index, &input_frames, &output_frames, &input_frame_index]
    (std::vector<std::complex<float>*> data, uint32_t size) {
      // Set the frame into the input
      SetTensorFrame(data, size, input, frame_index);
      
      if (frame_index == T - 1) {  // everything is filled up !
        // run
        auto status = bundle->session->Run(
            {{"Placeholder", input}}, GetOutputNames(separation_type), {}, &output);
        ASSERT_TRUE(status.ok());
        
        // copy the inputs
        CopyTensorBlock(input, T - FrameLatency, FrameLength, &input_frames);
        
        // and the outputs mask
        CopyTensorBlock(output[0], T - FrameLatency, FrameLength, &output_frames);
        
        // shift the input data of StepLength
        for (auto source_index = T - FrameLatency; source_index < T; source_index++) {
          auto destination_index = source_index - StepLength;
          if (destination_index < 0) {
            continue;
          }
          // Copy frame source_index into destination_index
          MoveTensorFrame(input, source_index, destination_index, size, data.size());
        }
        // start to fill again
        frame_index = T - StepLength;
      } else {
        frame_index += 1;
      }
      
      // convert the output !
      // TODO: deal with overlap !
      for (auto channel_index = 0; channel_index < data.size(); channel_index++) {
        Eigen::Map<Eigen::VectorXcf> output(data[channel_index], size);
        auto fft_frame = input_frames[channel_index].col(input_frame_index);
        auto mask_frame = output_frames[channel_index].col(input_frame_index);
        
        output = fft_frame.array() * mask_frame.array();
      }
      
      input_frame_index += 1;
      if (input_frame_index >= FrameLength) {
        input_frame_index = 0;
      }
  };

  // Run each frames
  auto multichannel_buffer_size = block_size * channel_number;
  for (uint32_t sample_idx = 0;
     sample_idx < data.size() - multichannel_buffer_size;
     sample_idx += multichannel_buffer_size) {
//    std::cout << sample_idx << " " << data.size() << std::endl;
    // process the input buffer
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
