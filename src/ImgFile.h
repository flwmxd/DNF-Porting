#pragma once
#include "Npk.h"
#include <assert.h>

class NpkFile;

struct ImgNode
{
	NpkTexture texture;

	uint32_t format;	//目前已知的类型有 0x0E(1555格式) 0x0F(4444格式) 0x10(8888格式) 0x11(不包含任何数据，可能是指内容同上一帧)
	int32_t linkId;

	struct {
		int32_t keep;
		int32_t mapIndex;
		int32_t left;
		int32_t top;
		int32_t right;
		int32_t bottom;
		int32_t rotate;
	}zlibInfo;

	uint32_t offset;

	bool isLink;
	//
	std::string uniqueName;
	NpkFile* reader = nullptr;
	auto getData()->const std::vector<uint8_t>&;
	std::vector<uint8_t> input;
};

class ImgFile
{
public:

	static ImgFile nullNode;
	ImgFile(NpkFile* file);

	inline auto isValid() const { return this != &nullNode; }
	inline operator bool() const { return isValid(); }
	auto unpack() -> std::string;
	auto openColorBoard(std::vector<uint32_t>& color) -> void;
	auto openMapImages(int32_t size) -> void;
	inline auto getFileName() const -> std::string { return metaInfo.fileName; }
	auto expand() -> void;
	inline auto& operator[](int32_t index) { assert(index < nodes.size() && index >= 0); return nodes[index]; }
private:
	NpkIndex metaInfo;
	ImgHeader header;
	NpkFile* file = nullptr;
	bool oldVersion = false;

	std::vector<ImgNode> nodes;
	std::vector<uint32_t> colorBoard;
	std::vector<std::vector<uint32_t>> colorBoards;
	std::vector<MapImage> mapImages;
	std::vector<int32_t> mapImagesOffset;
	
};