import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import network
from esphome import automation
from esphome.const import (
    CONF_ID, CONF_PORT, CONF_PASSWORD, CONF_SSID,
    CONF_TRIGGER_ID, CONF_ON_MESSAGE, CONF_MESSAGE,
    CONF_SERVICE, CONF_TARGET, CONF_DATA
)

DEPENDENCIES = ["network"]
CODEOWNERS = ["@your-github-username"]

painlessmesh_ns = cg.esphome_ns.namespace("painlessmesh")
PainlessMeshComponent = painlessmesh_ns.class_(
    "PainlessMeshComponent", cg.Component, network.NetworkListener
)

# 消息接收触发器
MessageReceivedTrigger = painlessmesh_ns.class_(
    "MessageReceivedTrigger",
    automation.Trigger.template(cv.std_string, cv.uint32)
)

# 发送动作
SendMessageAction = painlessmesh_ns.class_(
    "SendMessageAction",
    automation.Action
)

# 配置参数
CONF_ON_MESSAGE = "on_message"
CONF_NODE_ID = "node_id"
CONF_BROADCAST = "broadcast"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(PainlessMeshComponent),
    cv.Required(CONF_SSID): cv.string,
    cv.Required(CONF_PASSWORD): cv.string,
    cv.Optional(CONF_PORT, default=5555): cv.port,
    cv.Optional(CONF_ON_MESSAGE): automation.validate_automation({
        cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(MessageReceivedTrigger),
        cv.Optional(CONF_MESSAGE): cv.string,
    }),
}).extend(cv.COMPONENT_SCHEMA)

# 发送消息动作定义
@automation.register_action(
    "painlessmesh.send",
    SendMessageAction,
    cv.Schema({
        cv.GenerateID(): cv.use_id(PainlessMeshComponent),
        cv.Optional(CONF_NODE_ID, default=0): cv.uint32_t,
        cv.Required(CONF_MESSAGE): cv.templatable(cv.string),
        cv.Optional(CONF_BROADCAST, default=False): cv.boolean,
    })
)
def painlessmesh_send_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    yield cg.register_parented(var, config[CONF_ID])
    
    if config[CONF_BROADCAST]:
        cg.add(var.set_broadcast(True))
    else:
        cg.add(var.set_target(config[CONF_NODE_ID]))
    
    templ = yield cg.templatable(config[CONF_MESSAGE], args, cv.std_string)
    cg.add(var.set_message(templ))
    yield var

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    
    cg.add(var.set_ssid(config[CONF_SSID]))
    cg.add(var.set_password(config[CONF_PASSWORD]))
    cg.add(var.set_port(config[CONF_PORT]))
    
    for conf in config.get(CONF_ON_MESSAGE, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        if CONF_MESSAGE in conf:
            cg.add(trigger.set_filter(conf[CONF_MESSAGE]))
        yield automation.build_automation(
            trigger, [(cv.std_string, "message"), (cv.uint32, "from")], conf
        )
    
    yield network.register_network(var)
