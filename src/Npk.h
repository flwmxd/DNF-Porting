#pragma once

#include <cstdint>
#include <vector>
#include <string>

#define MAX_FILENAME_LENGTH 256
#define MAX_HEADER_FLAG 16




struct NpkHeader 
{
	char flag[MAX_HEADER_FLAG]; //"NeoplePack_Bill"
	int32_t count;     // 包内文件的数目
};

struct NpkIndex
{
	uint32_t offset;
	uint32_t size;
	char fileName[MAX_FILENAME_LENGTH];//256;//MAX_FILENAME_LENGTH
};


struct ImgHeader
{
	char flag[MAX_HEADER_FLAG + 2]; //"Neople Img File"
	int32_t indexSize;// 索引表大小，以字节为单位
	int32_t keep;
	int32_t version;
	int32_t indexCount;// 索引表数目
};

struct NpkTextureOffset
{
	//目前已知的类型有 0x0E(1555格式) 0x0F(4444格式) 0x10(8888格式) 0x11(不包含任何数据，可能是指内容同上一帧)
	uint32_t format;
	uint32_t compress; //0x06(zlib压缩) 0x05(未压缩) (0x07) map_zlib 
	//offset in file
	uint32_t offset;
	uint32_t bufferSize;
};

struct NpkTexture
{
	int32_t extra;
	int32_t width;
	int32_t height;
	int32_t size;// 压缩时size为压缩后大小，未压缩时size为转换成8888格式时占用的内存大小
	int32_t x;
	int32_t y;
	int32_t maxWidth;
	int32_t maxWeight;
};

enum BitmapType
{
	ARGB_1555 = 14,
	ARGB_4444 = 15,
	ARGB_8888 = 16,
	ARGB_LINK = 17,
	Format_DXT_1 = 18,
	Format_DXT_3 = 19,
	Format_DXT_5 = 20
};

enum CompressType
{
	COMPRESS_NONE = 0x05,
	COMPRESS_ZLIB = 0x06,
	COMPRESS_MAP_ZLIB = 0x07,
};

struct MapImage
{
	int32_t keep;
	int32_t fmt;
	int32_t index;
	int32_t dataSize;
	int32_t rawSize;
	int32_t w;
	int32_t h;
};
