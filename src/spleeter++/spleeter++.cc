#include "spleeter++/spleeter++.h"
#include <memory>

#include <pybind11/embed.h>

namespace spleeter {

namespace internal {
/// A container for the python interpreter
class PythonInterpreter {
public:
  static PythonInterpreter &instance() {
    static PythonInterpreter instance;
    return instance;
  }
  void Start() {
    interpreter_ = std::make_shared<pybind11::scoped_interpreter>();
  }
  void Stop() { interpreter_.reset(); }

private:
  PythonInterpreter() {}
  ~PythonInterpreter() { Stop(); }
  std::shared_ptr<pybind11::scoped_interpreter> interpreter_;
};
} // namespace internal

void Initialize(const std::string &virtual_environment_path) {
  internal::PythonInterpreter::instance().Start();
  // update the path variable
  pybind11::exec("import os");
  pybind11::exec("path = os.environ['PATH'].split(':')");
  pybind11::exec("path.append('" + virtual_environment_path + "/bin')");
  pybind11::exec("os.environ['PATH'] = ':'.join(path)");

  pybind11::exec(
      "import tempfile"); // we rely on tempfile to create output directories
  pybind11::exec("import shutil"); // we rely on shutil to remove temp directory

  // Import the separator
  pybind11::exec("from spleeter.separator import Separator");
}

void Separate(const std::string &audio_file_path,
              const std::string &vocals_path,
              const std::string &accompaniment_path) {
  pybind11::exec("output_path = tempfile.mkdtemp('spleeter')");

  // Initialize the separator
  pybind11::exec("separator = Separator('spleeter:2stems')");
  pybind11::exec("source_file = '" + audio_file_path + "'");

  // Run it !
  pybind11::exec("separator.separate_to_file(source_file, output_path)");

  // Move output files to the right location
  pybind11::exec(
      "source_filename = os.path.splitext(os.path.basename(source_file))[0]");
  pybind11::exec(
      "output_audio_path = os.path.join(output_path, source_filename)");
  pybind11::exec(
      "shutil.move(os.path.join(output_audio_path, 'vocals.wav'), '" +
      vocals_path + "')");
  pybind11::exec("shutil.move(os.path.join(output_audio_path, "
                 "'accompaniment.wav'), '" +
                 accompaniment_path + "')");

  pybind11::exec("shutil.rmtree(output_path)"); // cleanup
}

} // namespace spleeter
