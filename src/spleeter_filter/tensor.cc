#include "spleeter_filter/tensor.h"

namespace spleeter {

TFHandlePtr<TF_Tensor> TensorAlloc(TF_DataType type,
                                   std::vector<int64_t> shapes) {
  size_t sample_count = 0;
  for (const auto &shape : shapes) {
    if (sample_count == 0) {
      sample_count = shape;
      continue;
    }
    sample_count *= shape;
  }

  // Allocate memory
  size_t data_len = 0;
  void *data = nullptr;
  void (*dealloc)(void *, size_t, void *) = nullptr;
  switch (type) {
  case TF_DataType::TF_FLOAT:
    data = internal::DataAlloc<float>(sample_count, &data_len);
    dealloc = internal::DataDealloc<float>;
    break;
  case TF_DataType::TF_COMPLEX:
    data = internal::DataAlloc<std::complex<float>>(sample_count, &data_len);
    dealloc = internal::DataDealloc<std::complex<float>>;
    break;
  default:
    assert(false); // Unsupported data type
  }

  // Create the tensor
  auto tensor_ptr = TF_NewTensor(type, shapes.data(), shapes.size(), data,
                                 data_len, dealloc, nullptr);
  return MakeHandle(tensor_ptr, TF_DeleteTensor);
}

}  // namespace spleeter
