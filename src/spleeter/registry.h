#ifndef SPLEETER_REGISTRY_H_
#define SPLEETER_REGISTRY_H_

#include "tensorflow/cc/saved_model/loader.h"
#include "spleeter/tf_handle.h"
#include "spleeter/spleeter.h"

namespace spleeter {

class Registry {
 public:
  using BundlePtr = std::pair<TFHandlePtr<TF_Session>, TFHandlePtr<TF_Graph>>;

  static Registry& instance();
  void Register(BundlePtr bundle, SeparationType type);
  BundlePtr Get(SeparationType type);

 private:
  Registry();
  ~Registry();

 private:
  std::map<SeparationType, BundlePtr> m_registry;
};

}  // namespace spleeter

#endif  // SPLEETER_REGISTRY_H_
