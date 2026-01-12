#include "lsltemplate/Device.hpp"
#include <chrono>
#include <thread>

namespace lsltemplate {

// =============================================================================
// MockDevice Implementation
// =============================================================================

MockDevice::MockDevice(const Config& config)
    : config_(config)
    , counter_(config.start_value)
{
}

MockDevice::~MockDevice() {
    disconnect();
}

bool MockDevice::connect() {
    // In a real implementation, initialize hardware connection here
    connected_ = true;
    counter_ = config_.start_value;
    return true;
}

void MockDevice::disconnect() {
    connected_ = false;
}

bool MockDevice::isConnected() const {
    return connected_;
}

DeviceInfo MockDevice::getInfo() const {
    return {
        .name = config_.name,
        .type = config_.type,
        .channel_count = config_.channel_count,
        .sample_rate = config_.sample_rate,
        .source_id = config_.name + "_mock"
    };
}

bool MockDevice::getData(std::vector<float>& buffer) {
    if (!connected_) {
        return false;
    }

    // Calculate samples based on buffer size and channel count
    const size_t samples_requested = buffer.size() / config_.channel_count;

    // Generate synthetic data
    for (size_t i = 0; i < buffer.size(); ++i) {
        buffer[i] = static_cast<float>(counter_++);
    }

    // Simulate real-time acquisition by sleeping
    if (config_.sample_rate > 0) {
        const auto sample_period = std::chrono::duration<double>(
            samples_requested / config_.sample_rate
        );
        std::this_thread::sleep_for(
            std::chrono::duration_cast<std::chrono::milliseconds>(sample_period)
        );
    }

    return true;
}

} // namespace lsltemplate
