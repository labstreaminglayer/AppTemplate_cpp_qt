#pragma once
/**
 * @file StreamThread.hpp
 * @brief Background thread for LSL streaming
 *
 * Manages the acquisition loop in a separate thread.
 */

#include "Device.hpp"
#include "LSLOutlet.hpp"
#include <atomic>
#include <functional>
#include <memory>
#include <thread>

namespace lsltemplate {

/**
 * @brief Status callback type
 */
using StatusCallback = std::function<void(const std::string& message, bool is_error)>;

/**
 * @brief Manages device acquisition and LSL streaming in a background thread
 */
class StreamThread {
public:
    /**
     * @brief Construct a stream thread for the given device
     * @param device Device to stream from (takes ownership)
     * @param callback Optional status callback for notifications
     */
    explicit StreamThread(
        std::unique_ptr<IDevice> device,
        StatusCallback callback = nullptr
    );

    ~StreamThread();

    // Non-copyable, non-movable (owns a running thread)
    StreamThread(const StreamThread&) = delete;
    StreamThread& operator=(const StreamThread&) = delete;

    /**
     * @brief Start streaming
     * @return true if started successfully
     */
    bool start();

    /**
     * @brief Stop streaming
     *
     * Signals the thread to stop and waits for it to finish.
     */
    void stop();

    /// Check if currently streaming
    bool isRunning() const;

    /// Get the device info
    DeviceInfo getDeviceInfo() const;

private:
    void threadFunction();

    std::unique_ptr<IDevice> device_;
    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> shutdown_{false};
    StatusCallback statusCallback_;
};

} // namespace lsltemplate
