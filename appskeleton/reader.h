#pragma once
#include <cstdint>
#include <vector>

class Reader {
public:
	explicit Reader(int32_t device_param);
	~Reader() noexcept;
	bool getData(std::vector<int32_t> &buffer);
	bool getStatus() { return true; }

private:
	int32_t counter;
};
