#include "ImgFile.h"
#include "NpkFile.h"
#include <zlib.h>
#include <iostream>

const char* FileNameFlag = "puchikon@neople dungeon and fighter DNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNFDNF";
const char* ImgFlag = "Neople Img File";
const char* ImgFlag2 = "Neople Image File";


constexpr uint8_t table4[0x10] = {
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
};

constexpr uint8_t  table5[0x20] = {
	0x00, 0x08, 0x10, 0x19, 0x21, 0x29, 0x31, 0x3A, 0x42, 0x4A, 0x52,
	0x5A, 0x63, 0x6B, 0x73, 0x7B, 0x84, 0x8C, 0x94, 0x9C, 0xA5, 0xAD,
	0xB5, 0xBD, 0xC5, 0xCE, 0xD6, 0xDE, 0xE6, 0xEF, 0xF7, 0xFF
};

//2^6 = 64
constexpr uint8_t table6[0x40] = {
	0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C, 0x20, 0x24, 0x28, 0x2D, 0x31, 0x35, 0x39, 0x3D,
	0x41, 0x45, 0x49, 0x4D, 0x51, 0x55, 0x59, 0x5D, 0x61, 0x65, 0x69, 0x6D, 0x71, 0x75, 0x79, 0x7D,
	0x82, 0x86, 0x8A, 0x8E, 0x92, 0x96, 0x9A, 0x9E, 0xA2, 0xA6, 0xAA, 0xAE, 0xB2, 0xB6, 0xBA, 0xBE,
	0xC2, 0xC6, 0xCA, 0xCE, 0xD2, 0xD7, 0xDB, 0xDF, 0xE3, 0xE7, 0xEB, 0xEF, 0xF3, 0xF7, 0xFB, 0xFF
};


ImgFile ImgFile::nullNode(nullptr);

ImgFile::ImgFile(NpkFile* file):file(file)
{

}

auto ImgFile::unpack() -> std::string
{
	file->readBytes(reinterpret_cast<uint8_t*>(&metaInfo), sizeof(NpkIndex));
	//TODO use SIMD later
	for (int i = 0; i < MAX_FILENAME_LENGTH; ++i) {
		metaInfo.fileName[i] = metaInfo.fileName[i] ^ FileNameFlag[i];
	}
	return metaInfo.fileName;
}

auto ImgFile::openColorBoard(std::vector<uint32_t>& color) -> void
{
	auto count = file->read<int32_t>();
	color.resize(count);
	file->readBytes((uint8_t*)color.data(), sizeof(uint32_t) * count);
}

auto ImgFile::openMapImages(int32_t size) -> void
{
	mapImages.resize(size);
	file->readBytes((uint8_t*)mapImages.data(), sizeof(MapImage) * size);
}

auto ImgFile::expand() -> void
{

	file->setPosition(metaInfo.offset);
	file->readBytes((uint8_t*)header.flag, 18);
	bool ok = false;

	if (strcmp(header.flag, ImgFlag2) == 0) {
		//Old Client Version
		ok = true;
		oldVersion = true;
		header.indexSize = file->read<uint16_t>();
	}

	if (!ok) {
		if (strcmp(header.flag, ImgFlag) == 0) {
			ok = true;
			file->setPosition(file->getOffset() - 2);
			header.indexSize = file->read<int32_t>();
		}
	}

	if (!ok) {
		printf(" Invalid file %s \n", metaInfo.fileName);
		return;
	}

	header.keep =		file->read<int32_t>();
	header.version =	file->read<int32_t>();
	header.indexCount = file->read<int32_t>();

	printf(" %s : Version %d \n", metaInfo.fileName, header.version);

	switch (header.version) {
	case 4:
		openColorBoard(colorBoard);
		break;
	case 5:
	{
		auto mapCount = file->read<int32_t>();
		auto fileSize = file->read<int32_t>();
		openColorBoard(colorBoard);
		openMapImages(mapCount);
	}
	break;
	case 6://multiple color board.
	{
		int32_t colorBoardCount = file->read<int32_t>();
		auto& color = colorBoards.emplace_back();
		for (auto i = 0; i < colorBoardCount; i++) {
			openColorBoard(color);
		}
	}
	break;
	}

	nodes.resize(header.indexCount);

	for (auto i = 0; i < header.indexCount; i++)
	{
		auto& imgNode = nodes[i];
		imgNode.reader = file;
		imgNode.uniqueName = metaInfo.fileName + std::to_string(i);
		file->read<uint32_t>(imgNode.format);
		imgNode.isLink = imgNode.format == BitmapType::ARGB_LINK;
		if (imgNode.isLink)
		{
			file->read<int32_t>(imgNode.linkId);
		}
		else
		{
			imgNode.linkId = -1;

			file->read<NpkTexture>(imgNode.texture);
			if ((header.version == 1 || header.version == 2) && imgNode.texture.extra == CompressType::COMPRESS_NONE){
				imgNode.texture.size = imgNode.texture.width * imgNode.texture.height * (imgNode.format == ARGB_8888 ? 4 : 2);
			}

			if (imgNode.texture.extra == CompressType::COMPRESS_MAP_ZLIB) {
				file->readBytes((uint8_t*)&imgNode.zlibInfo, sizeof(imgNode.zlibInfo));
			}

			if (header.version == 1) {
				imgNode.offset = file->getOffset();
				file->setPosition(file->getOffset() + imgNode.texture.size);
			}
		}
	}

	if (header.version != 1)
	{
		auto offset = file->getOffset();
		if (header.version == 2) {
		//	file->setPosition(metaInfo.offset + header.indexSize + 32);
			offset = metaInfo.offset + header.indexSize + 32;
		}

		if (header.version == 5)
		{
			mapImagesOffset.resize(mapImages.size());
			for (int32_t i = 0; i < mapImages.size();i++) {
				mapImagesOffset[i] = offset;
				offset += mapImages[i].dataSize;
			}
		}

		for (auto & image:  nodes)
		{
			if (image.format != BitmapType::ARGB_LINK && image.texture.extra != COMPRESS_MAP_ZLIB)
			{
				image.offset = offset;
				offset += image.texture.size;
			}
		}
	}

	for (auto& n : nodes)
	{
		if (n.isLink)
		{
			n.texture = nodes[n.linkId].texture;
			n.offset = nodes[n.linkId].offset;
			n.zlibInfo = nodes[n.linkId].zlibInfo;
		}
	}
}

struct color8888
{
	uint8_t r : 8;
	uint8_t g : 8;
	uint8_t b : 8;
	uint8_t a : 8;
};

struct color4444
{
	uint8_t a : 4;
	uint8_t r : 4;
	uint8_t g : 4;
	uint8_t b : 4;
};

/*
struct color1555
{
	uint8_t a : 1;
	uint8_t r : 5;
	uint8_t g : 5;
	uint8_t b : 5;
};*/


static auto convert1555To8888(std::vector<uint8_t>& input, std::vector<uint8_t>& output, int32_t pixels) -> void
{
	auto argb1555 = reinterpret_cast<uint16_t*>(input.data());
	auto pixelsout = reinterpret_cast<color8888*>(output.data());
	for (auto i = 0; i < pixels; ++i)
	{
		auto& c = argb1555[i];
		//16 bit
		uint16_t a = c & 0x8000, r = c & 0x7C00, g = c & 0x03E0, b = c & 0x1F;
		a = (a >> 15) & 0x1f;
		r = (r >> 10) & 0x1f;
		g = (g >> 5) & 0x1f;
		b = b;

		pixelsout[i] = { table5[r], table5[g], table5[b], (uint8_t)(a == 1 ? 0xFF : 0) };
	}
	input.swap(output);
}


static auto convert4444To8888(std::vector<uint8_t> & input, std::vector<uint8_t>& output, int32_t pixels) -> void 
{
	auto pixels4444 = reinterpret_cast<color4444*>(input.data());
	auto pixelsout = reinterpret_cast<color8888*>(output.data());
	for (auto i = 0; i < pixels; ++i)
	{
		auto p = pixels4444[i];
		pixelsout[i] = { table4[p.r], table4[p.g], table4[p.b], table4[p.a] };
	}
	input.swap(output);
}


auto ImgNode::getData() ->const std::vector<uint8_t> &
{
	if (!input.empty()) {
		return input;
	}
	input.resize(texture.size);
	reader->setPosition(offset);
	reader->readBytes(input.data(),texture.size);
	if (texture.extra == CompressType::COMPRESS_ZLIB) {
		std::vector<uint8_t> output;
		output.resize(texture.width * texture.height * (format == ARGB_8888 ? 4 : 2));
		unsigned long size = output.size();
		auto err = uncompress(output.data(), &size, input.data(), input.size());
		if (err != Z_OK)
		{
			std::cerr << "uncompess error: " << err << '\n';
		}
		input.swap(output);
		if (format == ARGB_8888) {
			return input;
		}

		output.resize(texture.width * texture.height * 4);

		if (format == ARGB_4444) {
			convert4444To8888(input, output, texture.width * texture.height);
		}
		else if (format == ARGB_1555) 
		{
			convert1555To8888(input, output, texture.width * texture.height);
		}
		return input;
	}
	return input;
}
