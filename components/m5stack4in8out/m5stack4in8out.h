#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/core/hal.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome::m5stack4in8out {


static constexpr uint8_t MODULE_4IN8OUT_ADDR            { 0x45 };
static constexpr uint8_t MODULE_4IN8OUT_INPUT_REG       { 0x10 };
static constexpr uint8_t MODULE_4IN8OUT_OUTPUT_REG      { 0x20 };
static constexpr uint8_t MODULE_4IN8OUT_VERSION_REG     { 0xFE };
static constexpr uint8_t MODULE_4IN8OUT_ADDR_CONFIG_REG { 0xFF };
static constexpr uint8_t i2c_address = 0x45;

enum class out_bit_e : uint8_t {
  out_1 = 0,
  out_2 = 1,
  out_3 = 2,
  out_4 = 3,
  out_5 = 4,
  out_6 = 5,
  out_7 = 6,
  out_8 = 7,
  unknown = 255
};

enum class in_bit_e : uint8_t {
  in_1 = 0,
  in_2 = 1,
  in_3 = 2,
  in_4 = 3,
  unknown = 255
};

class m5stack4in8out_switch;

// ====================================================================================

class m5stack4in8out : public i2c::I2CDevice, public Component {
 public:
  void setup() override;
  void dump_config() override;
  bool set_output(out_bit_e bit, bool state);
  bool get_input(in_bit_e bit, bool& state);
};

// ====================================================================================

class m5stack4in8out_switch : public switch_::Switch, public Component, public Parented<m5stack4in8out> {
 public:
  m5stack4in8out_switch(out_bit_e bit) : bit_(bit) {}

  void set_assumed_state(bool assumed_state) { this->assumed_state_ = assumed_state; };

 protected:
  bool assumed_state() override { return this->assumed_state_; };

  void write_state(bool state) override;

  bool assumed_state_{false};

  out_bit_e bit_{out_bit_e::unknown};
};

class m5stack4in8out_binarysensor : public PollingComponent, public binary_sensor::BinarySensor, public Parented<m5stack4in8out> {
public:
  m5stack4in8out_binarysensor(in_bit_e bit) : PollingComponent(100), bit_(bit) {}

  void update() override;

protected:
  in_bit_e bit_{in_bit_e::unknown};
};



}
