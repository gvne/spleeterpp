#ifndef SPEETER_SPLEETER_H_
#define SPEETER_SPLEETER_H_

#include <string>
#include <memory>
#include <unordered_set>
#include <Eigen/Core>

namespace spleeter {

using Waveform = Eigen::Matrix2Xf;

enum SeparationType { TwoStems, FourStems, FiveStems };

void Initialize(const std::string &path_to_models,
                const std::unordered_set<SeparationType> &separation_type,
                std::error_code &err);

void Split(const Waveform &input, Waveform *vocals, Waveform *accompaniment,
           std::error_code &err);

 void Split(const Waveform &input, Waveform *vocals, Waveform *drums,
           Waveform *bass, Waveform *other, std::error_code& err);

 void Split(const Waveform &input, Waveform *vocals, Waveform *drums,
           Waveform *bass, Waveform *piano, Waveform *other, std::error_code& err);

} // namespace spleeter

#endif // SPEETER_SPLEETER_H_
