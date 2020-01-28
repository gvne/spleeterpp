#ifndef SPLEETER_TYPE_H_
#define SPLEETER_TYPE_H_

#include <Eigen/Core>

namespace spleeter {

enum SeparationType { TwoStems, FourStems, FiveStems };

using Waveform = Eigen::Matrix2Xf;

}  // namespace spleeter

#endif  // SPLEETER_TYPE_H_
