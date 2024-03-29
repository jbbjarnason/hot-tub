//
// Created by jonb on 3/29/24.
//

#ifndef MY_CLIMATE_HPP
#define MY_CLIMATE_HPP

#include "esphome.h"
#include "esphome/core/component.h"
#include "esphome/components/servo/servo.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/climate/climate_traits.h"
#include "esphome/components/climate/climate_mode.h"

namespace stx {
  template< class T, class... Args >
  constexpr T* construct_at( T* p, Args&&... args ) {
    return ::new (static_cast<void*>(p)) T(std::forward<Args>(args)...);
  }
}

template <typename storage_t, std::size_t len>
struct circular_buffer {
  circular_buffer() = default;

  /// \param args arguments to forward to constructor of storage_t
  /// \return removed item, the oldest item
  template <typename... Args>
  constexpr auto emplace(Args&&... args) -> storage_t {
    // TODO this move is not working for non copyable types
    storage_t removed_item{ std::move(*insert_pos_) };
    front_ = insert_pos_;
    stx::construct_at(insert_pos_, std::forward<decltype(args)>(args)...);
    std::advance(insert_pos_, 1);
    if (insert_pos_ == std::end(buffer_)) {
      insert_pos_ = std::begin(buffer_);
    }
    return std::move(removed_item);
  }
  /// \return reference to most recently inserted item
  constexpr auto front() noexcept -> storage_t& { return *front_; }
  /// \return const reference to most recently inserted item
  constexpr auto front() const noexcept -> storage_t const& { return *front_; }
  /// \return reference to oldest inserted item
  constexpr auto back() noexcept -> storage_t& { return *insert_pos_; }
  /// \return const reference to oldest inserted item
  constexpr auto back() const noexcept -> storage_t const& { return *insert_pos_; }

  /// \brief Access the n-th item from the front
  /// example: buffer[0] is the most recently inserted item
  /// buffer[1] is the second most recently inserted item
  /// buffer[len - 1] is the oldest inserted item
  constexpr auto operator[](std::size_t n) const noexcept -> storage_t const& {
    // assert(n < len && "Index out of bounds");
    typename std::array<storage_t, len>::const_iterator front{ front_ };
    auto idx{ std::distance(std::cbegin(buffer_), front) };
    idx -= n;
    if (idx < 0) {
      idx += len;
    }
    // assert(idx >= 0 && "Something weird occured");
    return buffer_[static_cast<std::size_t>(idx)];
  }

  constexpr auto size() const noexcept -> std::size_t { return buffer_.size(); }

  std::array<storage_t, len> buffer_{};
  // front is invalid when there has no item been inserted yet, but should not matter much
  typename std::array<storage_t, len>::iterator front_{ std::begin(buffer_) };
  typename std::array<storage_t, len>::iterator insert_pos_{ std::begin(buffer_) + 1 };  // this is front + 1
};

class MyCustomClimate : public Component, public Climate {
public:
  void setup() override {
    // This will be called by App.setup()
  }
  void control(const ClimateCall &call) override {
    if (call.get_mode().has_value()) {
      // User requested mode change
      ClimateMode mode = *call.get_mode();
      // Send mode to hardware
      // ...

      // Publish updated state
      this->mode = mode;
      this->publish_state();
    }
    if (call.get_target_temperature().has_value()) {
      // User requested target temperature change
      float temp = *call.get_target_temperature();
      // Send target temp to climate
      // ...
    }
  }
  ClimateTraits traits() override {
    // The capabilities of the climate device
    auto traits = climate::ClimateTraits();
    traits.set_supports_current_temperature(true);
    traits.set_supported_modes({climate::CLIMATE_MODE_HEAT_COOL});
    return traits;
  }


  sensor::Sensor *sensor_{ nullptr };
  servo::Servo* servo_{ nullptr };
  float increment_{ 0.01f }; // default 1%, each step is 1% of the voltage range
  std::uint32_t settling_time_{ 5000 }; // default 5 seconds, results in 500 seconds for calibration
  float settling_deviation_{ 0.1f }; // default 0.1 degree, settling deviation means the temperature must stay within this range for the settling time
  float deadband_{ 0.25f }; // default 0.25 degree, deadband means the temperature can be within this range of the target temperature
  float clamp_{ 50.0f }; // default 50.0 degree, clamp means the temperature can't be higher than this value, react quickly if it is
  // state
  float last_level_{ 0.0f };
  float target_temperature_{ 38.0f };
  circular_buffer<float, 4> last_temperatures_{};
};

#endif //MY_CLIMATE_HPP
