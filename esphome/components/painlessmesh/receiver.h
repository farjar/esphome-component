#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "painlessmesh.h"

namespace esphome {
namespace painlessmesh {

/**
 * @brief 消息接收处理器，支持多种消息处理模式
 */
class MeshMessageReceiver : public Component {
 public:
  // 注册消息回调（原始消息）
  void add_raw_handler(std::function<void(uint32_t from, const std::string &msg)> handler) {
    raw_handlers_.push_back(handler);
  }

  // 注册带过滤器的回调
  void add_filtered_handler(const std::string &filter, 
                          std::function<void(uint32_t from, const std::string &msg)> handler) {
    filtered_handlers_[filter] = handler;
  }

  // 处理接收到的消息
  void handle_message(uint32_t from, const std::string &msg) {
    // 原始消息处理
    for (auto &handler : raw_handlers_) {
      handler(from, msg);
    }

    // 过滤处理
    for (auto &[filter, handler] : filtered_handlers_) {
      if (msg.find(filter) != std::string::npos) {
        handler(from, msg);
      }
    }

    // 触发自动化
    if (this->trigger_ != nullptr) {
      this->trigger_->trigger(msg, from);
    }
  }

  // 设置自动化触发器
  void set_trigger(automation::Trigger<std::string, uint32_t> *trigger) {
    this->trigger_ = trigger;
  }

 protected:
  std::vector<std::function<void(uint32_t, const std::string &)>> raw_handlers_;
  std::map<std::string, std::function<void(uint32_t, const std::string &)>> filtered_handlers_;
  automation::Trigger<std::string, uint32_t> *trigger_ = nullptr;
};

/**
 * @brief 消息类型模板处理器（支持JSON/Protobuf等）
 */
template<typename T>
class TypedMessageReceiver : public MeshMessageReceiver {
 public:
  void add_handler(std::function<void(uint32_t from, const T &data)> handler) {
    typed_handlers_.push_back(handler);
  }

  void handle_typed_message(uint32_t from, const std::string &msg) {
    T data;
    if (this->parser_(msg, data)) {
      for (auto &handler : typed_handlers_) {
        handler(from, data);
      }
    }
  }

  void set_parser(std::function<bool(const std::string &, T &)> parser) {
    parser_ = parser;
  }

 private:
  std::vector<std::function<void(uint32_t, const T &)>> typed_handlers_;
  std::function<bool(const std::string &, T &)> parser_;
};

}  // namespace painlessmesh
}  // namespace esphome
