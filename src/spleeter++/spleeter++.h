#include <cstdint>
#include <string>

namespace spleeter {

/// Initialize the Spleeter environement
/// \param virtual_environment_path the path to the conda generated spleeter environment
void Initialize(const std::string& virtual_environment_path);

/// Separate into two stems
/// \param audio_file_path the source file path
/// \param vocals_path the extracted vocals path
/// \param accompaniment_path the remaining accompaniment path
void Separate(const std::string& audio_file_path,
              const std::string& vocals_path,
              const std::string& accompaniment_path);

/// Separate into four stems
/// \param audio_file_path the source file path
/// \param vocals_path the extracted vocals path
/// \param drums_path the extracted drums path
/// \param bass_path the extracted bass path
/// \param other_path the path to the remaining audio content
void Separate(const std::string& audio_file_path,
              const std::string& vocals_path,
              const std::string& drums_path,
              const std::string& bass_path,
              const std::string& other_path);

/// Separate into five stems
/// \param audio_file_path the source file path
/// \param vocals_path the extracted vocals path
/// \param drums_path the extracted drums path
/// \param bass_path the extracted bass path
/// \param piano_path the extracted piano path
/// \param other_path the path to the remaining audio content
void Separate(const std::string& audio_file_path,
              const std::string& vocals_path,
              const std::string& drums_path,
              const std::string& bass_path,
              const std::string& piano_path,
              const std::string& other_path);

}  // namespace spleeter
