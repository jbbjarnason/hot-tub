#include <chrono>
#include <cstdint>

#include "esphome/core/component.h"
#include "esphome/components/servo/servo.h"

namespace esphome::stable_control {

struct voltage {
  voltage() = default;
  // min 0.0, max 1.0
  voltage(float val) : value{ val } {}
  float value{};
  operator float() const { return value; }
};

struct temperature {
  temperature() = default;
  temperature(float val) : value{ val } {}
  float value{};
  operator float() const { return value; }
};

struct state {
  state() = default;
  state(temperature temp) : temperature{ temp } {}
  temperature temperature{};
  std::chrono::time_point<std::chrono::system_clock> timestamp{ std::chrono::system_clock::now() };
};

template <typename storage_t, std::size_t len>
struct circular_buffer {
  circular_buffer() = default;

  /// \param args arguments to forward to constructor of storage_t
  /// \return removed item, the oldest item
  constexpr auto emplace(auto&&... args) -> storage_t {
    // TODO this move is not working for non copyable types
    storage_t removed_item{ std::move(*insert_pos_) };
    front_ = insert_pos_;
    std::construct_at(insert_pos_, std::forward<decltype(args)>(args)...);
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
    auto idx{ std::ranges::distance(std::cbegin(buffer_), front_) };
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

static constexpr char const* const TAG = "stable_control";

class stable_control : public Component {

public:
  void setup() override {
    // config set output::FloatOutput
  }

  void dump_config() override;

  void increment() {
    if (this->last_level_ + this->increment_ > 1.0f) {
      ESP_LOGW(TAG, "Output level is already at maximum, not incrementing.");
      this->output_->set_level(1.0f);
      this->last_level_ = 1.0f;
      return;
    }
    this->last_level_ += this->increment_;
    this->output_->set_level(this->last_level_);
    ESP_LOGD(TAG, "Incrementing output level to: %f", this->last_level_);
  }

  void decrement() {
    if (this->last_level_ - this->increment_ < 0.0f) {
      ESP_LOGW(TAG, "Output level is already at minimum, not decrementing.");
      this->output_->set_level(0.0f);
      this->last_level_ = 0.0f;
      return;
    }
    this->last_level_ -= this->increment_;
    this->output_->set_level(this->last_level_);
    ESP_LOGD(TAG, "Decrementing output level to: %f", this->last_level_);
  }

  void on_temperature_change(float temp) {
    this->last_temperatures_.emplace(temp);
    if (temp > this->clamp_) {
      ESP_LOGW(TAG, "Temperature is higher than clamp value, decrementing output.");
      decrement();
      return;
    }
    // calculate last temperatures average
    float avg{ 0.0f };
    for (std::size_t i{ 0 }; i < this->last_temperatures_.size(); ++i) {
      avg += this->last_temperatures_[i];
    }
    // check if all last temperatures are within the settling deviation from average
    for (std::size_t i{ 0 }; i < this->last_temperatures_.size(); ++i) {
      if (std::abs(this->last_temperatures_[i] - avg) > this->settling_deviation_) {
        ESP_LOGD(TAG, "Temperature %f is not within settling deviation %f from average %f, waiting.", this->last_temperatures_[i], this->settling_deviation_, avg);
        return;
      }
    }
    // check if within deadband
    if (std::abs(temp - this->target_temperature_) < this->deadband_) {
      ESP_LOGD(TAG, "Temperature %f is within deadband, not changing output.", temp);
      return;
    }
    // check if temp is higher than target
    if (temp > this->target_temperature_) {
      ESP_LOGD(TAG, "Temperature %f is higher than target %f, decrementing output.", temp, this->target_temperature_);
      decrement();
    }
    // check if temp is lower than target
    else if (temp < this->target_temperature_) {
      ESP_LOGD(TAG, "Temperature %f is lower than target %f, incrementing output.", temp, this->target_temperature_);
      increment();
    }
    // let's clear the last temperatures buffer, that will require at least buffer size measurements until we can change the output again
    this->last_temperatures_ = {};
  }
  void set_target_temperature(float target_temperature) {
    this->target_temperature_ = target_temperature;

  }


protected:
  // config
  output::FloatOutput* output_{ nullptr };
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

}  // namespace esphome::stable_control
