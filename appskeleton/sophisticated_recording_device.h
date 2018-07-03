#ifndef SOPHISTICATED_RECORDING_DEVICE_H
#define SOPHISTICATED_RECORDING_DEVICE_H
#include <cstdint>
#include <vector>


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
