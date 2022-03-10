#pragma once

#include <cstdint>
#include <vector>
#include <string>

#define MAX_FILENAME_LENGTH 256
#define MAX_HEADER_FLAG 16




struct NpkHeader 
{
	char flag[MAX_HEADER_FLAG]; //"NeoplePack_Bill"
	int32_t count;     // �����ļ�����Ŀ
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
	int32_t indexSize;// �������С�����ֽ�Ϊ��λ
	int32_t keep;
	int32_t version;
	int32_t indexCount;// ��������Ŀ
};

struct NpkTextureOffset
{
	//Ŀǰ��֪�������� 0x0E(1555��ʽ) 0x0F(4444��ʽ) 0x10(8888��ʽ) 0x11(�������κ����ݣ�������ָ����ͬ��һ֡)
	uint32_t format;
	uint32_t compress; //0x06(zlibѹ��) 0x05(δѹ��) (0x07) map_zlib 
	//offset in file
	uint32_t offset;
	uint32_t bufferSize;
};

struct NpkTexture
{
	int32_t extra;
	int32_t width;
	int32_t height;
	int32_t size;// ѹ��ʱsizeΪѹ�����С��δѹ��ʱsizeΪת����8888��ʽʱռ�õ��ڴ��С
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
