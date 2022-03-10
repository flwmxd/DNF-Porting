#pragma once
#include <cstdint>
#include <string>
#include <stdio.h>
#include <memory>

#include "PvfNode.h"
#include <functional>
#include <mutex>

enum EncodingType
{
	TW = 950,
	CN = 936,
	KR = 949,
	JP = 932,
	UTF8 = 65001,
	Unicode = 1200
};



class PvfReader 
{
	struct PvfHeader
	{
		int32_t sizeGUID; //Always 0x24
		uint8_t GUID[0x24];
		int32_t fileVersion;
		int32_t dirTreeLength;//头文件占用字节大小
		int32_t dirTreeChecksum;//CRC32码
		int32_t numFilesInDirTree;//PVF文件总数
	};


public:
	friend class PvfDocument;

	static constexpr uint32_t PASSWORD_PVF = 0x81A79011;
	//static constexpr char* ENCODING = "CP949";
	static constexpr char* ENCODING = "BIG5HKSCS";
	PvfReader(const std::string& path);
	~PvfReader();
	auto unpack() -> void;
	auto setPosition(uint64_t pos) -> void;
	auto decrypt(uint8_t* ptr, uint32_t len, uint32_t crc32) -> void;
	auto readBytes(uint32_t length)->std::unique_ptr<uint8_t[]>;
	auto decryptString(const std::unique_ptr<uint8_t[]>& buffer, int32_t len, std::string& out) -> void;
	auto codeConvert(const char* fromCharset, const char* toCharset, const char* inbuf, size_t inlen, char* outbuf, size_t outlen)->int32_t;
	auto write(const std::string& file, const std::string & str) -> void;

	auto operator[](const std::string & path) ->PvfTreeNode&;

	inline auto& getRoot()  { return root; }
	inline auto isLoaded() const { return loaded; }

	template <typename T>
	// Read a number and advance the buffer position.
	inline T read(const uint8_t* buffer, int32_t offset)
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

	auto unpackStringTable(const std::function<void(
			std::function<void*()>, std::function<void(void*)>, int32_t
		)> & addTask, const std::function<void()> & waitAll
	) -> void;



private:
	auto dfsCreateNode(PvfNode& tag, PvfTreeNode* tree, const std::vector<std::string>& pathes, int32_t deep) -> void;
	auto mapping() -> void;



	int64_t length = 0;
	uint64_t pos = 0;
	FILE* file = nullptr;
	std::string path;
	PvfHeader header;

	std::unordered_map<std::string, PvfNode> pvfNodes;
	std::unordered_map<std::string, std::string> stringStringMap;
	std::vector<std::string> stringBinMap;

	
	PvfTreeNode root;
	bool loaded = false;
};
