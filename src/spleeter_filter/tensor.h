#ifndef SPLEETER_FILTER_TENSOR_H_
#define SPLEETER_FILTER_TENSOR_H_

#include <cassert>
#include <vector>
#include <complex>
#include <cstring>
#include "spleeter_common/tf_handle.h"

namespace spleeter {

namespace internal {

template <typename T> void *DataAlloc(size_t len, size_t *data_len) {
  auto ptr = new T[len];
  memset(ptr, 0, len * sizeof(T));
  *data_len = len * sizeof(T);
  return static_cast<void *>(ptr);
}

template <typename T> void DataDealloc(void *ptr, size_t len, void *) {
  auto data = reinterpret_cast<T *>(ptr);
  delete[] data;
}

template <typename T> class Adapter {
public:
  Adapter(TFHandlePtr<TF_Tensor> tensor, std::vector<int64_t> shapes)
      : m_tensor(tensor), m_shapes(shapes), m_frame_index(0), m_bin_index(1),
        m_channel_index(2) {}

  T &operator()(size_t frame_index, size_t bin_index, size_t channel_index) {
    auto ptr = static_cast<T *>(TF_TensorData(m_tensor->get()));
    // Data organization was found empirically:
    // [Bin 0 Channel 0 Frame 0]
    // [Bin 0 Channel 1 Frame 0]
    // [Bin 1 Channel 0 Frame 0]
    // [Bin 1 Channel 1 Frame 0]
    // ...
    // [Bin N Channel 0 Frame 0]
    // [Bin N Channel 1 Frame 0]
    // [Bin 0 Channel 0 Frame 1]
    // [Bin 0 Channel 1 Frame 1]
    // ...
    // [Bin N Channel 0 Frame M]
    // [Bin N Channel 1 Frame M]
    return ptr[(frame_index * (bin_count() * channel_count())) +
               (bin_index * (channel_count())) + (channel_index * (1))];
  }

  int64_t frame_count() const { return m_shapes[m_frame_index]; }
  int64_t bin_count() const { return m_shapes[m_bin_index]; }
  int64_t channel_count() const { return m_shapes[m_channel_index]; }

private:
  TFHandlePtr<TF_Tensor> m_tensor;
  std::vector<int64_t> m_shapes;
  int64_t m_frame_index;
  int64_t m_bin_index;
  int64_t m_channel_index;
};

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
  internal::Adapter<T> adapter(tensor, shapes);
  for (auto bin_index = 0; bin_index < adapter.bin_count(); bin_index++) {
    for (auto channel_index = 0; channel_index < adapter.channel_count();
         channel_index++) {
      adapter(frame_index, bin_index, channel_index) =
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
              const TFHandlePtr<TF_Tensor> tensor,
              std::vector<int64_t> shapes) {
  internal::Adapter<T> adapter(tensor, shapes);
  for (auto bin_index = 0; bin_index < adapter.bin_count(); bin_index++) {
    for (auto channel_index = 0; channel_index < data->size();
         channel_index++) {
      (*data)[channel_index][bin_index] =
          adapter(frame_index, bin_index, channel_index);
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
  internal::Adapter<T> adapter(tensor, shapes);
  for (auto bin_index = 0; bin_index < adapter.bin_count(); bin_index++) {
    for (auto channel_index = 0; channel_index < adapter.channel_count();
         channel_index++) {
      adapter(destination_index, bin_index, channel_index) =
          adapter(source_index, bin_index, channel_index);
    }
  }
}

/// Copy a frame into a vector of data pointers
/// \tparam T The type of data the tensor holds
/// \param source
/// \param shapes the tensor shapes
/// \param destination
template <typename T>
void Copy(const TF_Tensor *source, std::vector<int64_t> shapes,
          TFHandlePtr<TF_Tensor> destination) {
  size_t sample_count = 0;
  for (const auto &shape : shapes) {
    if (sample_count == 0) {
      sample_count = shape;
      continue;
    }
    sample_count *= shape;
  }
  auto src = static_cast<const T *>(TF_TensorData(source));
  auto dst = static_cast<T *>(TF_TensorData(destination->get()));

  memcpy(dst, src, sample_count * sizeof(T));
}

} // namespace spleeter

#endif // SPLEETER_FILTER_TENSOR_H_
