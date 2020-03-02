#ifndef SPLEETER_REGISTRY_H_
#define SPLEETER_REGISTRY_H_

#include "tensorflow/cc/saved_model/loader.h"
#include "spleeter/spleeter.h"

namespace spleeter {

using BundlePtr = std::shared_ptr<tensorflow::SavedModelBundle>;

class Registry {
 public:
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
