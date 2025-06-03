#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "painlessmesh.h"

namespace esphome {
namespace painlessmesh {

class SendMessageAction : public Action<std::string> {
 public:
  explicit SendMessageAction(PainlessMeshComponent *parent) : parent_(parent) {}
  
  void set_target(uint32_t target) { target_ = target; }
  void set_broadcast(bool broadcast) { broadcast_ = broadcast; }
  void set_message(const std::string &message) { message_ = message; }
  
  void play(const std::string &x) override {
    parent_->send_message(target_, message_.empty() ? x : message_, broadcast_);
  }

 protected:
  PainlessMeshComponent *parent_;
  uint32_t target_{0};
  bool broadcast_{false};
  std::string message_;
};

}  // namespace painlessmesh
}  // namespace esphome
