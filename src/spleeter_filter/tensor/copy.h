//#ifndef SPLEETER_TENSOR_COPY_H_
//#define SPLEETER_TENSOR_COPY_H_
//
//#include "tensorflow/cc/framework/ops.h"
//
//namespace spleeter {
//namespace tensor {
//
///// Copy an fft frame into to tensorflow tensor
///// \tparam The type of data the tensor holds
///// \param tensor
///// \param frame_index the destination frame index in the tensor
///// \param data the fft frame data
//template <typename T>
//void SetFrame(tensorflow::Tensor *tensor, uint32_t frame_index,
//              std::vector<T *> data) {
//  auto bin_size = tensor->shape().dim_size(1);
//
//  auto eigen_input = tensor->tensor<T, 3>();
//  for (auto bin_index = 0; bin_index < bin_size; bin_index++) {
//    for (auto channel_index = 0; channel_index < data.size(); channel_index++) {
//      eigen_input(frame_index, bin_index, channel_index) =
//          data[channel_index][bin_index];
//    }
//  }
//}
//
///// Copy a frame into a vector of data pointers
///// \tparam The type of data the tensor holds
///// \param data the output fft frame data
///// \param frame_index the source frame index in the tensor
///// \param tensor
//template <typename T>
//void GetFrame(std::vector<T *> *data, uint32_t frame_index,
//              const tensorflow::Tensor &tensor) {
//  auto bin_size = tensor.shape().dim_size(1);
//
//  auto eigen_input = tensor.tensor<T, 3>();
//  for (auto bin_index = 0; bin_index < bin_size; bin_index++) {
//    for (auto channel_index = 0; channel_index < data->size();
//         channel_index++) {
//      (*data)[channel_index][bin_index] =
//          eigen_input(frame_index, bin_index, channel_index);
//    }
//  }
//}
//
///// Move a tensor frame
///// \param tensor
///// \param source_index the source frame index
///// \param destination_index the destination frame index
//void MoveFrame(tensorflow::Tensor &tensor, uint16_t source_index,
//               uint16_t destination_index);
//
//}  // namespace tensor
//}  // namespace spleeter
//
//#endif // SPLEETER_TENSOR_COPY_H_
