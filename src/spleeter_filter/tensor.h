#ifndef SPLEETER_FILTER_TENSOR_H_
#define SPLEETER_FILTER_TENSOR_H_

#include "spleeter_filter/tensor_helper.h"

namespace spleeter {

template <typename T> class Tensor {
public:
  Tensor(const std::vector<int64_t> &shapes)
      : m_shapes(shapes),
        m_handler(TensorAlloc(TensorType<T>::data_type(), shapes)) {}

  void SetFrame(int64_t frame_index, const std::vector<const T *> &data) {
    spleeter::SetFrame(m_handler, m_shapes, frame_index, data);
  }

  void GetFrame(int64_t frame_index, const std::vector<T *> &data) {
    spleeter::GetFrame(data, frame_index, m_handler, m_shapes);
  }

  void MoveFrame(int64_t source, int64_t destination) {
    spleeter::MoveFrame<T>(m_handler, source, destination, m_shapes);
  }

  void Copy(const TF_Tensor *source) {
    spleeter::Copy<T>(source, m_shapes, m_handler);
  }

  TF_Tensor *ptr() { return m_handler->get(); }

private:
  std::vector<int64_t> m_shapes;
  TFHandlePtr<TF_Tensor> m_handler;
};

template <typename T> using TensorPtr = std::shared_ptr<Tensor<T>>;

template <typename T> class TensorFrame {
public:
  TensorFrame(uint8_t channel_count, uint64_t frame_size) {
    for (auto channel_idx = 0; channel_idx < channel_count; channel_idx++) {
      m_data.emplace_back(std::vector<T>(frame_size, 0));
      m_data_ptr.push_back(m_data[channel_idx].data());
      m_const_data_ptr.push_back(m_data[channel_idx].data());
    }
  }

  std::vector<T *> data() { return m_data_ptr; }

  std::vector<const T *> const_data() const { return m_const_data_ptr; }

private:
  std::vector<std::vector<T>> m_data;
  std::vector<T *> m_data_ptr;
  std::vector<const T *> m_const_data_ptr;
};

template <typename T> using TensorFramePtr = std::shared_ptr<TensorFrame<T>>;

} // namespace spleeter

#endif // SPLEETER_FILTER_TENSOR_H_

