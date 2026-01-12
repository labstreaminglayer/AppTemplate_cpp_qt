#pragma once
/**
 * @file LSLOutlet.hpp
 * @brief LSL stream outlet wrapper
 *
 * Provides a clean interface for creating and managing LSL stream outlets.
 */

#include "Device.hpp"
#include <lsl_cpp.h>
#include <memory>
#include <string>
#include <vector>

namespace lsltemplate {

/**
 * @brief Wrapper for LSL stream outlet
 *
 * Creates and manages an LSL outlet based on device information.
 * Handles stream creation, data pushing, and cleanup.
 */
class LSLOutlet {
public:
    /**
     * @brief Construct an outlet for the given device
     * @param info Device information for stream setup
     */
    explicit LSLOutlet(const DeviceInfo& info);

    ~LSLOutlet();

    // Non-copyable
    LSLOutlet(const LSLOutlet&) = delete;
    LSLOutlet& operator=(const LSLOutlet&) = delete;

    // Movable
    LSLOutlet(LSLOutlet&&) noexcept = default;
    LSLOutlet& operator=(LSLOutlet&&) noexcept = default;

    /**
     * @brief Push a chunk of samples to the outlet
     * @param data Channel-interleaved sample data
     */
    void pushChunk(const std::vector<float>& data);

    /**
     * @brief Push a single sample to the outlet
     * @param sample Single sample (one value per channel)
     */
    void pushSample(const std::vector<float>& sample);

    /// Get the stream name
    std::string getStreamName() const;

    /// Check if outlet has consumers
    bool hasConsumers() const;

private:
    std::unique_ptr<lsl::stream_outlet> outlet_;
    DeviceInfo info_;
};

} // namespace lsltemplate
