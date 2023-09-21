import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch, i2c
from esphome.const import CONF_ID, CONF_NAME, CONF_ASSUMED_STATE

DEPENDENCIES = ["i2c"]

m5stack4in8out_ns = cg.esphome_ns.namespace("m5stack4in8out")
m5stack4in8out = m5stack4in8out_ns.class_("m5stack4in8out", cg.Component, i2c.I2CDevice)
m5stack4in8out_switch = m5stack4in8out_ns.class_("m5stack4in8out_switch", switch.Switch, cg.Component)

CONF_OUTPUT1 = "out_1"
CONF_OUTPUT2 = "out_2"
CONF_OUTPUT3 = "out_3"
CONF_OUTPUT4 = "out_4"
CONF_OUTPUT5 = "out_5"
CONF_OUTPUT6 = "out_6"
CONF_OUTPUT7 = "out_7"
CONF_OUTPUT8 = "out_8"

OutputBit_ = m5stack4in8out_ns.enum("out_bit_e", is_class=True)
ENUM_COMP_SWITCHES = {
    CONF_OUTPUT1: OutputBit_.out_1,
    CONF_OUTPUT2: OutputBit_.out_2,
    CONF_OUTPUT3: OutputBit_.out_3,
    CONF_OUTPUT4: OutputBit_.out_4,
    CONF_OUTPUT5: OutputBit_.out_5,
    CONF_OUTPUT6: OutputBit_.out_6,
    CONF_OUTPUT7: OutputBit_.out_7,
    CONF_OUTPUT8: OutputBit_.out_8,
}


def check_relayswitch():
    return cv.maybe_simple_value(
        switch.switch_schema(m5stack4in8out_switch).extend(
            {cv.Optional(CONF_ASSUMED_STATE): cv.boolean}
        ),
        key=CONF_NAME,
    )


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(m5stack4in8out),
            cv.Optional(CONF_OUTPUT1): check_relayswitch(),
            cv.Optional(CONF_OUTPUT2): check_relayswitch(),
            cv.Optional(CONF_OUTPUT3): check_relayswitch(),
            cv.Optional(CONF_OUTPUT4): check_relayswitch(),
            cv.Optional(CONF_OUTPUT5): check_relayswitch(),
            cv.Optional(CONF_OUTPUT6): check_relayswitch(),
            cv.Optional(CONF_OUTPUT7): check_relayswitch(),
            cv.Optional(CONF_OUTPUT8): check_relayswitch(),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x45))
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