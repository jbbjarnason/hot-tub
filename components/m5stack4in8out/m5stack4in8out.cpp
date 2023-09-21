#include "esphome/core/log.h"
#include "m5stack4in8out.h"

namespace esphome::m5stack4in8out {

static char const *const TAG = "m5stack4in8out";

void m5stack4in8out::dump_config() { ESP_LOGCONFIG(TAG, "M5Stack 4in8out"); }

void m5stack4in8out::setup() {
  // todo availability to recover from eeprom
  set_output(out_bit_e::out_1, false);
  set_output(out_bit_e::out_2, false);
  set_output(out_bit_e::out_3, false);
  set_output(out_bit_e::out_4, false);
  set_output(out_bit_e::out_5, false);
  set_output(out_bit_e::out_6, false);
  set_output(out_bit_e::out_7, false);
  set_output(out_bit_e::out_8, false);

  ESP_LOGCONFIG(TAG, "Setting up M5STACK 4in8out (0x%02X)...", this->address_);
}

bool m5stack4in8out::set_output(out_bit_e bit, bool state) {
  uint8_t data = state ? 0x01 : 0x00;
  return write_byte(MODULE_4IN8OUT_OUTPUT_REG + static_cast<uint8_t>(bit), data);
}

bool m5stack4in8out::get_input(in_bit_e bit, bool& state) {
  uint8_t data{};
  auto result_code{ read_byte(MODULE_4IN8OUT_INPUT_REG + static_cast<uint8_t>(bit), &data) };
  state = static_cast<bool>(data);
  return result_code;
}

void m5stack4in8out_switch::write_state(bool state) {
  if (!this->parent_->set_output(this->bit_, state)) {
    publish_state(false);
  } else {
    publish_state(state);
  }
}

void m5stack4in8out_binarysensor::update() {
  bool new_state{};
  if (this->parent_ && this->parent_->get_input(this->bit_, new_state)) {
    if (this->state != new_state) {
      ESP_LOGCONFIG(TAG, "Input: %d, update state to: %d", static_cast<uint8_t>(this->bit_), new_state);
      publish_state(new_state);
      this->state = new_state;
    }
  }
  else {
    ESP_LOGCONFIG(TAG, "Unable to update state");
  }
}


}
