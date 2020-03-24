#include "spleeter_common/tf_handle.h"

namespace spleeter {

void SessionDeleter(TF_Session* ptr) {
  auto status = MakeHandle(TF_NewStatus(), TF_DeleteStatus);
  TF_DeleteSession(ptr, status->get());
  if (TF_GetCode(status->get()) != TF_Code::TF_OK) {
    std::cerr << "Failed to release the session handle" << std::endl;
  }
}

}  // namespace spleeter
