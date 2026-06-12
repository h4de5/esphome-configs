#pragma once
#include "esphome/core/component.h"
#include "esphome/components/api/api_server.h"
#include "esphome/components/text/text.h"
#include <map>
#include <vector>
#include <string>

namespace esphome {
namespace ha_entity_cache {

class HAEntityCache : public Component {
 public:
  void set_target_text(text::Text *target) { target_ = target; }
  void add_attribute(const std::string &attribute) { attributes_.push_back(attribute); }

  float get_setup_priority() const override { return -100.0f; }

  void setup() override {
    std::string entity_id = target_->state;
    if (entity_id.empty()) return;

    for (auto &attribute : attributes_) {
      esphome::optional<std::string> attr_opt;
      if (!attribute.empty()) attr_opt = attribute;

      api::global_api_server->subscribe_home_assistant_state(
          entity_id, attr_opt,
          [this, attribute](StringRef state) {
            this->cache_[attribute] = static_cast<std::string>(state);
          });
    }
  }

  std::string get(const std::string &attribute, const std::string &fallback) {
    auto it = cache_.find(attribute);
    if (it == cache_.end() || it->second.empty()) return fallback;
    return it->second;
  }

 protected:
  text::Text *target_;
  std::vector<std::string> attributes_;
  std::map<std::string, std::string> cache_;
};

}  // namespace ha_entity_cache
}  // namespace esphome
