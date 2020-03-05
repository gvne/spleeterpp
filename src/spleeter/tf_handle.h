#ifndef SPLEETER_TF_HANDLE_H_
#define SPLEETER_TF_HANDLE_H_

#include <memory>
#include <functional>
#include <iostream>

#include <tensorflow/c/c_api.h>

namespace spleeter {

template <typename T>
class TFHandle {
public:
  TFHandle(T* ptr, std::function<void(T*)> deleter) : m_ptr(ptr), m_deleter(deleter) {}
  TFHandle(const TFHandle&) = delete;
  ~TFHandle() {
    m_deleter(m_ptr);
  }
  T* get() {
    return m_ptr;
  }
private:
  T* m_ptr;
  std::function<void(T*)> m_deleter;
};

template <typename T>
using TFHandlePtr = std::shared_ptr<TFHandle<T>>;

template <typename T>
TFHandlePtr<T> MakeHandle(T* ptr, void (deleter)(T*)) {
  return std::make_shared<TFHandle<T>>(ptr, deleter);
}

void SessionDeleter(TF_Session* ptr) {
  auto status = MakeHandle(TF_NewStatus(), TF_DeleteStatus);
  TF_DeleteSession(ptr, status->get());
  if (TF_GetCode(status->get()) != TF_Code::TF_OK) {
    std::cerr << "Failed to release the session handle" << std::endl;
  }
}

}  // namespace spleeter

#endif  // SPLEETER_TF_HANDLE_H_
