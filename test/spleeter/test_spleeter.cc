#include <gtest/gtest.h>

#include <iostream>
#include <unordered_set>
#include <complex>

#include <tensorflow/c/c_api.h>

template <typename T>
class TFHandle {
public:
  TFHandle(T* ptr, std::function<void(T*)> deleter) : m_ptr(ptr), m_deleter(deleter) {}
  ~TFHandle() {
    m_deleter(m_ptr);
  }
  T* get() {
    return m_ptr;
  }
private:
  T* m_ptr;
  std::function<void(T*)> m_deleter;
};

template <typename T>
TFHandle<T> MakeHandle(T* ptr, void (deleter)(T*)) {
  return TFHandle<T>(ptr, deleter);
}

void SessionDeleter(TF_Session* ptr) {
  auto status = MakeHandle(TF_NewStatus(), TF_DeleteStatus);
  TF_DeleteSession(ptr, status.get());
  if (TF_GetCode(status.get()) != TF_Code::TF_OK) {
    std::cerr << "Failed to release the session handle" << std::endl;
  }
}

template <typename T>
void* NewData(size_t len, size_t* output_size) {
  *output_size = len * sizeof(T);
  return static_cast<void*>(new T[len]);
}

template <typename T>
void DataDeleter(void* data, size_t len, void*) {
  auto float_data = static_cast<T*>(data);
  delete[] float_data;
}

TEST(Spleeter, Basic) {
  auto session_options = MakeHandle(TF_NewSessionOptions(), TF_DeleteSessionOptions);
  auto graph = MakeHandle(TF_NewGraph(), TF_DeleteGraph);
  auto run_options = MakeHandle(TF_NewBuffer(), TF_DeleteBuffer);
  auto meta_graph_def = MakeHandle(TF_NewBuffer(), TF_DeleteBuffer);
  auto status = MakeHandle(TF_NewStatus(), TF_DeleteStatus);
  std::vector<const char*> tags({"serve"});
    
  std::string two_stem_model_dir(SPLEETER_MODELS);
  two_stem_model_dir += "/2stems";
  
  // Initialize the session
  auto session_ptr = TF_LoadSessionFromSavedModel(session_options.get(), run_options.get(), two_stem_model_dir.c_str(), tags.data(), tags.size(), graph.get(), meta_graph_def.get(), status.get());
  ASSERT_TRUE(TF_GetCode(status.get()) == TF_Code::TF_OK);
  auto session = MakeHandle(session_ptr, SessionDeleter);
  
  // 10 seconds stereo
  std::vector<int64_t> input_dims = {10, 2049, 2};
  size_t data_len;
  auto data = NewData<std::complex<float>>(input_dims[0] * input_dims[1] * input_dims[2], &data_len);
  
  // Build input
  TF_Output input_op{TF_GraphOperationByName(graph.get(), "Placeholder"), 0};
  ASSERT_TRUE(input_op.oper);
  auto input_tensor_ptr = TF_NewTensor(TF_DataType::TF_COMPLEX, input_dims.data(),input_dims.size(), data, data_len, DataDeleter<std::complex<float>>, nullptr);
  auto input_tensor = MakeHandle(input_tensor_ptr, TF_DeleteTensor);
  std::vector<TF_Tensor*> inputs = {input_tensor.get()};
  
  // And the output
  TF_Output output_vocals_op{TF_GraphOperationByName(graph.get(), "strided_slice_11"), 0};
  ASSERT_TRUE(output_vocals_op.oper);
  TF_Output output_accompaniment_op{TF_GraphOperationByName(graph.get(), "strided_slice_19"), 0};
  ASSERT_TRUE(output_accompaniment_op.oper);
  std::vector<TF_Output> output_ops = {output_vocals_op, output_accompaniment_op};
  std::vector<TF_Tensor*> outputs = {nullptr, nullptr};

  TF_SessionRun(session.get(), run_options.get(), &input_op, inputs.data(), inputs.size(), output_ops.data(), outputs.data(), output_ops.size(), nullptr, 0, nullptr, status.get());
  ASSERT_TRUE(TF_GetCode(status.get()) == TF_Code::TF_OK);
  
//  auto data = static_cast<float*>(TF_TensorData(output_tensor));

  auto vocals = MakeHandle(outputs[0], TF_DeleteTensor);
  auto accompaniment = MakeHandle(outputs[1], TF_DeleteTensor);
  
  auto vocals_data = static_cast<float*>(TF_TensorData(vocals.get()));
  auto accompaniment_data = static_cast<float*>(TF_TensorData(accompaniment.get()));
}

// #include "wave/file.h"
// #include "spleeter/spleeter.h"
//
// void Write(const spleeter::Waveform& data, const std::string& name) {
//   std::vector<float> vec_data(data.size());
//   std::copy(data.data(), data.data() + data.size(), vec_data.data());
//   wave::File file;
//   file.Open(std::string(OUTPUT_DIR) + "/" + name + ".wav", wave::kOut);
//   file.set_sample_rate(44100);
//   file.set_channel_number(2);
//   file.Write(vec_data);
// }
//
//
// TEST(Spleeter, TwoStems) {
//   // Read wav file
//   wave::File file;
//   file.Open(std::string(TEST_FILE), wave::kIn);
//   std::error_code err;
//   auto data = file.Read(err);
//   ASSERT_FALSE(err);
//   auto source = Eigen::Map<spleeter::Waveform>(data.data(), 2, data.size() / 2);
//
//   // ------------------------------
//   // Spleeter !
//   spleeter::Initialize(
//       std::string(SPLEETER_MODELS),
//       {spleeter::TwoStems, spleeter::FourStems, spleeter::FiveStems}, err);
//   ASSERT_FALSE(err);
//   {
//     spleeter::Waveform vocals, accompaniment;
//     spleeter::Split(source, &vocals, &accompaniment, err);
//     ASSERT_FALSE(err);
//     Write(vocals, "vocals-2stems");
//     Write(accompaniment, "accompaniment-2stems");
//   }
//   {
//     spleeter::Waveform vocals, drums, bass, other;
//     spleeter::Split(source, &vocals, &drums, &bass, &other, err);
//     ASSERT_FALSE(err);
//     Write(vocals, "vocals-4stems");
//     Write(drums, "drums-4stems");
//     Write(bass, "bass-4stems");
//     Write(other, "other-4stems");
//   }
//   {
//     spleeter::Waveform vocals, drums, bass, piano, other;
//     spleeter::Split(source, &vocals, &drums, &bass, &piano, &other, err);
//     ASSERT_FALSE(err);
//     Write(vocals, "vocals-5stems");
//     Write(drums, "drums-5stems");
//     Write(bass, "bass-5stems");
//     Write(piano, "piano-5stems");
//     Write(other, "other-5stems");
//   }
// }
