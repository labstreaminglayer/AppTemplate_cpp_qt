# `sophisticated_recording_device.h` ([source](../appskeleton/sophisticated_recording_device.h))

``` cpp
#ifndef SOPHISTICATED_RECORDING_DEVICE_H
#define SOPHISTICATED_RECORDING_DEVICE_H
#include <cstdint>
#include <vector>
```

Most recording device APIs provide some sort of handle to a device and
functions to query the state, read data and put it in a buffer etc.

This is a very simple example to demonstrate how to integrate it with LSL.
The provided functions are:

- the constructor
- the destructor
- `getData` with the buffer as one output parameter and the status as return value
- `getStatus` to check if everything's ok

``` cpp
class sophisticated_recording_device {
public:
	explicit sophisticated_recording_device(int32_t device_param);
	~sophisticated_recording_device();
	bool getData(std::vector<int32_t>& buffer);
	bool getStatus() { return true; }
private:
	int32_t counter;
};
#endif // SOPHISTICATED_RECORDING_DEVICE_H
