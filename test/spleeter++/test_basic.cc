#include <gtest/gtest.h>
#include <pybind11/embed.h>
namespace py = pybind11;

TEST(Spleeter, Basic) {
  py::scoped_interpreter guard{};
  
  // initialize the interpreter
  py::exec("import os");
  py::exec("path = os.environ['PATH'].split(':')");
  py::exec("path.append('" + std::string(SPLEETER_ENV) + "/bin')");
  py::exec("os.environ['PATH'] = ':'.join(path)");
  
  // run spleeter
  py::exec("from spleeter.separator import Separator");
  py::exec("separator = Separator('spleeter:2stems')");
  py::exec("separator.separate_to_file('" + std::string(TEST_FILE_PATH) + "', '/tmp')");
}
