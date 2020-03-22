#include "spleeter/tensor/copy.h"

namespace spleeter {
namespace tensor {

//void MoveFrame(tensorflow::Tensor& tensor,
//                     uint16_t source_index,
//                     uint16_t destination_index) {
//  auto frame_size = tensor.shape().dim_size(1);
//  auto channel_count = tensor.shape().dim_size(2);
//
//  auto eigen_input = tensor.tensor<std::complex<float>, 3>();
//  for (auto bin_index = 0; bin_index < frame_size; bin_index++) {
//    for (auto channel_index = 0; channel_index < channel_count; channel_index++) {
//      eigen_input(destination_index, bin_index, channel_index) = \
//        eigen_input(source_index, bin_index, channel_index);
//    }
//  }
//}
}  // namespace tensor
}  // namespace spleeter
