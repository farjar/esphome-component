#include "painlessmesh.h"

namespace esphome {
namespace painlessmesh {

static const char *const TAG = "painlessmesh";

void PainlessMeshComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up PainlessMesh...");
  
  mesh_.init(ssid_.c_str(), password_.c_str(), port_);
  mesh_.onReceive([this](uint32_t from, String &msg) {
    this->handle_received(from, msg);
  });
  
  mesh_.setContainsRoot(true);
  mesh_.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
}

void PainlessMeshComponent::loop() {
  static uint32_t last_update = 0;
  if (millis() - last_update > 10) {  // 控制更新频率
    mesh_.update();
    last_update = millis();
  }
}

void PainlessMeshComponent::handle_received(uint32_t from, const String &msg) {
  std::string message(msg.c_str());
  ESP_LOGD(TAG, "Received from %u: %s", from, message.c_str());
  
  for (auto *trigger : triggers_) {
    trigger->process(from, message);
  }
}

void PainlessMeshComponent::register_trigger(MessageReceivedTrigger *trigger) {
  triggers_.push_back(trigger);
}

void PainlessMeshComponent::send_message(uint32_t target, const std::string &message, bool broadcast) {
  if (broadcast) {
    mesh_.sendBroadcast(message.c_str());
  } else {
    mesh_.sendSingle(target, message.c_str());
  }
  ESP_LOGD(TAG, "Sent to %s%u: %s", 
           broadcast ? "broadcast" : "", 
           broadcast ? 0 : target, 
           message.c_str());
}

std::vector<uint32_t> PainlessMeshComponent::get_nodes() const {
  std::vector<uint32_t> nodes;
  for (const auto &node : mesh_.getNodeList()) {
    nodes.push_back(node);
  }
  return nodes;
}

size_t PainlessMeshComponent::node_count() const {
  return mesh_.getNodeList().size();
}

void PainlessMeshComponent::on_wifi_disconnect() {
  ESP_LOGW(TAG, "WiFi disconnected, stopping mesh");
  mesh_.stop();
}

void PainlessMeshComponent::on_wifi_connect() {
  ESP_LOGI(TAG, "WiFi connected, restarting mesh");
  mesh_.init(ssid_.c_str(), password_.c_str(), port_);
}

void PainlessMeshComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "PainlessMesh:");
  ESP_LOGCONFIG(TAG, "  SSID: %s", ssid_.c_str());
  ESP_LOGCONFIG(TAG, "  Port: %d", port_);
  ESP_LOGCONFIG(TAG, "  Connected Nodes: %zu", this->node_count());
}

}  // namespace painlessmesh
}  // namespace esphome
