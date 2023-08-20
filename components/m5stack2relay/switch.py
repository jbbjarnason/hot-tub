import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch, i2c
from esphome.const import CONF_ID, CONF_NAME, CONF_ASSUMED_STATE

DEPENDENCIES = ["i2c"]

M5RELAY2_ns = cg.esphome_ns.namespace("m5stack2relay")
M5_relay_2 = M5RELAY2_ns.class_("M5_relay_2", cg.Component, i2c.I2CDevice)
M5_relay_2_switch = M5RELAY2_ns.class_("M5_relay_2_switch", switch.Switch, cg.Component)

CONF_RELAY1 = "relay_1"
CONF_RELAY2 = "relay_2"

RelayBit_ = M5RELAY2_ns.enum("relay_bit_e", is_class=True)
ENUM_COMP_SWITCHES = {
    CONF_RELAY1: RelayBit_.relay_1,
    CONF_RELAY2: RelayBit_.relay_2,
}


def check_relayswitch():
    return cv.maybe_simple_value(
        switch.switch_schema(M5_relay_2_switch).extend(
            {cv.Optional(CONF_ASSUMED_STATE): cv.boolean}
        ),
        key=CONF_NAME,
    )


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(M5_relay_2),
            cv.Optional(CONF_RELAY1): check_relayswitch(),
            cv.Optional(CONF_RELAY2): check_relayswitch(),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x25))
)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield i2c.register_i2c_device(var, config)

    for key, value in ENUM_COMP_SWITCHES.items():
        if key in config:
            conf = config[key]
            sens = yield switch.new_switch(conf, value)
            if CONF_ASSUMED_STATE in conf:
                cg.add(sens.set_assumed_state(conf[CONF_ASSUMED_STATE]))
            cg.add(sens.set_parent(var))