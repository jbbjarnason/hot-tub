#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "esphome/core/hal.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome::m5stack2relay {

static constexpr uint8_t MODULE_2RELAY_ADDR = 0x25;
static constexpr uint8_t MODULE_2RELAY_REG = 0x00;
static constexpr uint8_t MODULE_2RELAY_VERSION_REG = 0xFE;
static constexpr uint8_t MODULE_2RELAY_ADDR_CONFIG_REG = 0xFF;

enum class relay_bit_e : uint8_t {
  relay_1 = 0,
  relay_2 = 1,
  unknown = 255
};

class M5_relay_2_switch;

// ====================================================================================

class M5_relay_2 : public i2c::I2CDevice, public Component {
 public:
  void setup() override;
  void dump_config() override;
  bool set_relay(relay_bit_e bit, bool state);
};

// ====================================================================================

class M5_relay_2_switch : public switch_::Switch, public Component, public Parented<M5_relay_2> {
 public:
  M5_relay_2_switch(relay_bit_e bit) : bit_(bit) {}

  void set_assumed_state(bool assumed_state) { this->assumed_state_ = assumed_state; };

 protected:
  bool assumed_state() override { return this->assumed_state_; };

  void write_state(bool state) override;

  bool assumed_state_{false};

  relay_bit_e bit_{relay_bit_e::unknown};
};

}
