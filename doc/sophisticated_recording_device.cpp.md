# `sophisticated_recording_device.cpp` ([source](../appskeleton/sophisticated_recording_device.cpp))

``` cpp
#include "sophisticated_recording_device.h"
#include <chrono>
#include <thread>
sophisticated_recording_device::sophisticated_recording_device(int32_t device_param)
    : counter(device_param) {
	// Here we would connect to the device
}
sophisticated_recording_device::~sophisticated_recording_device() {
	// Close the connection
}
bool sophisticated_recording_device::getData(std::vector<int32_t>& buffer) {
	// Acquire some data and return it
	for (auto& elem : buffer) elem = counter++;
	std::this_thread::sleep_for(std::chrono::milliseconds(buffer.size() * 100));
	return true;
}
