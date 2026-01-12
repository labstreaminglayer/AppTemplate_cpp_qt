#include "lsltemplate/StreamThread.hpp"
#include <iostream>

namespace lsltemplate {

StreamThread::StreamThread(
    std::unique_ptr<IDevice> device,
    StatusCallback callback
)
    : device_(std::move(device))
    , statusCallback_(std::move(callback))
{
}

StreamThread::~StreamThread() {
    stop();
}

bool StreamThread::start() {
    if (running_) {
        return false;  // Already running
    }

    if (!device_) {
        if (statusCallback_) {
            statusCallback_("No device configured", true);
        }
        return false;
    }

    // Connect to device
    if (!device_->connect()) {
        if (statusCallback_) {
            statusCallback_("Failed to connect to device", true);
        }
        return false;
    }

    // Start the streaming thread
    shutdown_ = false;
    running_ = true;
    thread_ = std::make_unique<std::thread>(&StreamThread::threadFunction, this);

    if (statusCallback_) {
        statusCallback_("Streaming started", false);
    }

    return true;
}

void StreamThread::stop() {
    if (!running_) {
        return;
    }

    // Signal shutdown
    shutdown_ = true;

    // Wait for thread to finish
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
    thread_.reset();

    // Disconnect device
    if (device_) {
        device_->disconnect();
    }

    running_ = false;

    if (statusCallback_) {
        statusCallback_("Streaming stopped", false);
    }
}

bool StreamThread::isRunning() const {
    return running_;
}

DeviceInfo StreamThread::getDeviceInfo() const {
    if (device_) {
        return device_->getInfo();
    }
    return {};
}

void StreamThread::threadFunction() {
    try {
        // Create LSL outlet
        auto info = device_->getInfo();
        LSLOutlet outlet(info);

        if (statusCallback_) {
            statusCallback_("LSL outlet created: " + info.name, false);
        }

        // Allocate buffer for acquisition
        // Buffer size: enough for ~100ms of data, minimum 1 sample
        size_t samples_per_chunk = std::max(
            1,
            static_cast<int>(info.sample_rate * 0.1)
        );
        std::vector<float> buffer(samples_per_chunk * info.channel_count);

        // Acquisition loop
        while (!shutdown_) {
            if (device_->getData(buffer)) {
                outlet.pushChunk(buffer);
            } else {
                // getData returned false - device error or disconnection
                if (!shutdown_) {
                    if (statusCallback_) {
                        statusCallback_("Device acquisition error", true);
                    }
                }
                break;
            }
        }

    } catch (const std::exception& e) {
        if (statusCallback_) {
            statusCallback_(std::string("Streaming error: ") + e.what(), true);
        }
    }

    running_ = false;
}

} // namespace lsltemplate
