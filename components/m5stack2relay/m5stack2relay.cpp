#include "esphome/core/log.h"
#include "m5stack2relay.h"
#include <bitset>

static char const *const TAG = "m5stack_2relay.switch";

namespace esphome::m5stack2relay {

void M5_relay_2::dump_config() { ESP_LOGCONFIG(TAG, "M5Stack 2 relay switch"); }

void M5_relay_2::setup() {
  // todo availability to recover from eeprom
  set_relay(relay_bit_e::relay_1, false); 
  set_relay(relay_bit_e::relay_2, false); 

  ESP_LOGCONFIG(TAG, "Setting up M5STACK 2RELAY (0x%02X)...", this->address_);
}

bool M5_relay_2::set_relay(relay_bit_e bit, bool state) {
  uint8_t data = state ? 0xff : 0x00;
  return write_byte(MODULE_2RELAY_REG + static_cast<uint8_t>(bit), data);
}

void M5_relay_2_switch::write_state(bool state) {
  if (!this->parent_->set_relay(this->bit_, state)) {
    publish_state(false);
  } else {
    publish_state(state);
  }
}

}
