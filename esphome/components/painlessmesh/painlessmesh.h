#pragma once

#include "esphome/components/network/network.h"
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include <painlessMesh.h>
#include <map>
#include <functional>

namespace esphome {
namespace painlessmesh {

class MessageReceivedTrigger;

class PainlessMeshComponent : public Component, public network::NetworkListener {
 public:
  MeshMessageReceiver *make_receiver() {
    auto receiver = new MeshMessageReceiver();
    receivers_.push_back(receiver);
    return receiver;
  }

  template<typename T>
  TypedMessageReceiver<T> *make_typed_receiver() {
    auto receiver = new TypedMessageReceiver<T>();
    typed_receivers_.push_back(receiver);
    return receiver;
  }

 private:
  std::vector<MeshMessageReceiver*> receivers_;
  std::vector<void*> typed_receivers_;  // 类型擦除存储
};
 public:
  void set_ssid(const std::string &ssid) { ssid_ = ssid; }
  void set_password(const std::string &password) { password_ = password; }
  void set_port(uint16_t port) { port_ = port; }

  void setup() override;
  void loop() override;
  void dump_config() override;

  // 消息处理
  void register_trigger(MessageReceivedTrigger *trigger);
  void send_message(uint32_t target, const std::string &message, bool broadcast = false);

  // 网络状态
  void on_wifi_disconnect() override;
  void on_wifi_connect() override;

  // 节点信息
  std::vector<uint32_t> get_nodes() const;
  size_t node_count() const;

 protected:
  friend class MessageReceivedTrigger;
  
  void handle_received(uint32_t from, const String &msg);
  
  painlessMesh mesh_;
  std::string ssid_;
  std::string password_;
  uint16_t port_{5555};
  std::vector<MessageReceivedTrigger*> triggers_;
};

class MessageReceivedTrigger : public Trigger<std::string, uint32_t> {
 public:
  explicit MessageReceivedTrigger(PainlessMeshComponent *parent) : parent_(parent) {}
  void set_filter(const std::string &filter) { filter_ = filter; }
  void setup() override { parent_->register_trigger(this); }
  
  bool process(uint32_t from, const std::string &message) {
    if (filter_.empty() || message.find(filter_) != std::string::npos) {
      this->trigger(message, from);
      return true;
    }
    return false;
  }

 protected:
  PainlessMeshComponent *parent_;
  std::string filter_;
};

}  // namespace painlessmesh
}  // namespace esphome
