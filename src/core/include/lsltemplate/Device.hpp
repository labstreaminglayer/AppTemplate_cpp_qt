#pragma once
/**
 * @file Device.hpp
 * @brief Abstract device interface for LSL data acquisition
 *
 * This file defines the interface that device implementations should follow.
 * Replace the MockDevice implementation with your actual device SDK integration.
 */

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace lsltemplate {

/**
 * @brief Device information structure
 */
struct DeviceInfo {
    std::string name;           ///< Device name for LSL stream
    std::string type;           ///< Stream type (e.g., "EEG", "Markers", "Audio")
    int channel_count = 1;      ///< Number of channels
    double sample_rate = 0.0;   ///< Nominal sample rate (0 for irregular)
    std::string source_id;      ///< Unique source identifier
};

/**
 * @brief Abstract base class for device implementations
 *
 * Derive from this class to implement support for your specific device.
 * The key methods to implement are:
 *   - connect()    - Initialize connection to hardware
 *   - disconnect() - Clean up hardware connection
 *   - getData()    - Retrieve samples from the device
 *   - getInfo()    - Return device metadata for LSL stream creation
 */
class IDevice {
public:
    virtual ~IDevice() = default;

    /// Connect to the device. Returns true on success.
    virtual bool connect() = 0;

    /// Disconnect from the device.
    virtual void disconnect() = 0;

    /// Check if currently connected
    virtual bool isConnected() const = 0;

    /// Get device information for LSL stream setup
    virtual DeviceInfo getInfo() const = 0;

    /**
     * @brief Retrieve data from the device
     * @param buffer Output buffer to fill with samples (channel-interleaved)
     * @return true if data was retrieved successfully, false on error or shutdown
     *
     * This method should block until data is available or an error occurs.
     * The buffer size determines how many samples to retrieve.
     */
    virtual bool getData(std::vector<float>& buffer) = 0;
};

/**
 * @brief Mock device for testing and template demonstration
 *
 * This implementation generates synthetic data. Replace with your actual
 * device SDK integration.
 */
class MockDevice : public IDevice {
public:
    struct Config {
        std::string name = "MockDevice";
        std::string type = "Counter";
        int channel_count = 1;
        double sample_rate = 10.0;  // 10 Hz
        int32_t start_value = 0;
    };

    explicit MockDevice(const Config& config);
    ~MockDevice() override;

    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    DeviceInfo getInfo() const override;
    bool getData(std::vector<float>& buffer) override;

private:
    Config config_;
    bool connected_ = false;
    int32_t counter_ = 0;
};

} // namespace lsltemplate
