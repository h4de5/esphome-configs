import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import text

DEPENDENCIES = ["api", "text"]

ha_entity_cache_ns = cg.esphome_ns.namespace("ha_entity_cache")
HAEntityCache = ha_entity_cache_ns.class_("HAEntityCache", cg.Component)

CONF_TARGET = "target_text"
CONF_ATTRIBUTES = "attributes"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(HAEntityCache),
    cv.Required(CONF_TARGET): cv.use_id(text.Text),
    cv.Required(CONF_ATTRIBUTES): cv.ensure_list(cv.string),
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    target = await cg.get_variable(config[CONF_TARGET])
    cg.add(var.set_target_text(target))

    for attr in config[CONF_ATTRIBUTES]:
        cg.add(var.add_attribute(attr))
