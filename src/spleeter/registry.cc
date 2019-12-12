#include "spleeter/registry.h"

namespace spleeter {

Registry::Registry() {}
Registry::~Registry() {}
  
Registry& Registry::instance() {
  static Registry instance;
  return instance;
}

void Registry::Register(BundlePtr bundle, SeparationType type) {
  m_registry[type] = bundle;
}

Registry::BundlePtr Registry::Get(SeparationType type) {
  return m_registry.at(type);
}

}  // namespace spleeter
