#ifndef SPLEETER_TENSOR_H_
#define SPLEETER_TENSOR_H_

#include <cassert>
#include <unsupported/Eigen/CXX11/Tensor>

#include "spleeter/tf_handle.h"

namespace spleeter {

namespace internal {

template <typename T>
void *DataAlloc(size_t len, size_t *data_len) {
  auto ptr = new T[len];
  *data_len = len * sizeof(T);
  return static_cast<void *>(ptr);
}

template <typename T>
void DataDealloc(void *ptr, size_t len, void *) {
  auto data = reinterpret_cast<T *>(ptr);
  delete[] data;
}

} // namespace internal

/// Allocate a new tensor with the given shape and data type
/// \param type the tensorflow datatype. Note: only TF_FLOAT and TF_COMPLEX are
/// supported at the moment
/// \param shapes the desired shape
/// \return A handle to the tensor
TFHandlePtr<TF_Tensor> TensorAlloc(TF_DataType type,
                                   std::vector<int64_t> shapes);

/// Copy an fft frame into to tensorflow tensor
/// \tparam T The type of data the tensor holds
/// \param tensor
/// \param shapes the tensor shapes
/// \param frame_index the destination frame index in the tensor
/// \param data the fft frame data
template <typename T>
void SetFrame(TFHandlePtr<TF_Tensor> tensor, std::vector<int64_t> shapes,
              uint32_t frame_index, std::vector<T *> data) {

  auto ptr = static_cast<T *>(TF_TensorData(tensor->get()));
  auto eigen_input = Eigen::TensorMap<Eigen::Tensor<T, 3>>(
      ptr, shapes[0], shapes[1], shapes[2]);
  for (auto bin_index = 0; bin_index < shapes[1]; bin_index++) {
    for (auto channel_index = 0; channel_index < data.size(); channel_index++) {
      eigen_input(frame_index, bin_index, channel_index) =
          data[channel_index][bin_index];
    }
  }
}

/// Copy a frame into a vector of data pointers
/// \tparam T The type of data the tensor holds
/// \param data the output fft frame data
/// \param frame_index the source frame index in the tensor
/// \param tensor
/// \param shapes the tensor shapes
template <typename T>
void GetFrame(std::vector<T *> *data, uint32_t frame_index,
              const TFHandlePtr<TF_Tensor> tensor, std::vector<int64_t> shapes) {
  
  auto ptr = static_cast<T *>(TF_TensorData(tensor->get()));
  auto eigen_input = Eigen::TensorMap<Eigen::Tensor<T, 3>>(
      ptr, shapes[0], shapes[1], shapes[2]);
  for (auto bin_index = 0; bin_index < shapes[1]; bin_index++) {
    for (auto channel_index = 0; channel_index < data->size();
         channel_index++) {
      (*data)[channel_index][bin_index] =
          eigen_input(frame_index, bin_index, channel_index);
    }
  }
}

/// Move a tensor frame
/// \tparam T The type of data the tensor holds
/// \param tensor
/// \param source_index the source frame index
/// \param destination_index the destination frame index
template <typename T>
void MoveFrame(TFHandlePtr<TF_Tensor> tensor, uint32_t source_index,
               uint32_t destination_index, std::vector<int64_t> shapes) {
  auto ptr = static_cast<T *>(TF_TensorData(tensor->get()));
  auto eigen_input = Eigen::TensorMap<Eigen::Tensor<T, 3>>(
      ptr, shapes[0], shapes[1], shapes[2]);
  for (auto bin_index = 0; bin_index < shapes[1]; bin_index++) {
    for (auto channel_index = 0; channel_index < shapes[2]; channel_index++) {
      eigen_input(destination_index, bin_index, channel_index) = \
        eigen_input(source_index, bin_index, channel_index);
    }
  }
}

/// Copy a frame into a vector of data pointers
/// \tparam T The type of data the tensor holds
/// \param source
/// \param shapes the tensor shapes
/// \param destination
template <typename T>
void Copy(const TF_Tensor* source, std::vector<int64_t> shapes,
          TFHandlePtr<TF_Tensor> destination) {
  size_t sample_count = 0;
  for (const auto &shape : shapes) {
    if (sample_count == 0) {
      sample_count = shape;
      continue;
    }
    sample_count *= shape;
  }
  auto src = static_cast<const T*>(TF_TensorData(source));
  auto dst = static_cast<T*>(TF_TensorData(destination->get()));
  
  memcpy(dst, src, sample_count * sizeof(T));
}

} // namespace spleeter

#endif // SPLEETER_TENSOR_H_
