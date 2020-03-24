#ifndef SPLEETER_FILTER_REGISTRY_H_
#define SPLEETER_FILTER_REGISTRY_H_

#include <map>
#include "spleeter_common/tf_handle.h"
#include "spleeter_common/type.h"

namespace spleeter {

using Bundle = std::pair<TFHandlePtr<TF_Session>, TFHandlePtr<TF_Graph>>;
using BundlePtr = std::shared_ptr<Bundle>;

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

#endif  // SPLEETER_FILTER_REGISTRY_H_
