
#pragma once
#include <cstdint>
#include <string>

class BufferReader
{
public:
	BufferReader(const uint8_t* buffer, int32_t len) : buffer(buffer), len(len) {}
	template <typename T>
	inline T read()
	{
		size_t count = sizeof(T) / sizeof(int8_t);
		T all = 0;
		for (size_t i = 0; i < count; i++)
		{
			T val = static_cast<T>(buffer[offset]);
			all += val << (8 * i);
			offset++;
		}
		return static_cast<T>(all);
	}

	template<>
	inline float read()
	{
		int32_t v = read<int32_t>();
		return *reinterpret_cast<float*>(&v);
	}
	inline auto readAsciiString(int32_t len) -> std::string {
		std::string str = { buffer + offset ,buffer + offset + len };
		offset += len;
		return str;
	}

	inline auto getOffset() const { return offset; }
	inline auto setOffset(int32_t off) { offset = off; }

private:
	const uint8_t* buffer;
	int32_t len;
	int32_t offset = 0;
};
