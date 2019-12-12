#ifndef SPEETER_SPLEETER_H_
#define SPEETER_SPLEETER_H_

#include <string>
#include <memory>
#include <Eigen/Core>

namespace spleeter {

using Waveform = Eigen::Matrix2Xf;

enum SeparationType { TwoStems, FourStems, FiveStems };

template <SeparationType... type>
void Initialize(const std::string &path_to_models);

template <SeparationType type, typename... T>
void Split(const Waveform &input, T *... outputs);

void Split(const Waveform &input, Waveform *vocals, Waveform *accompaniment);
void Split(const Waveform &input, Waveform *vocals, Waveform *drums,
           Waveform *bass, Waveform *other);
void Split(const Waveform &input, Waveform *vocals, Waveform *drums,
           Waveform *bass, Waveform *piano, Waveform *other);

} // namespace spleeter

#include "spleeter/spleeter.hxx"

#endif // SPEETER_SPLEETER_H_
