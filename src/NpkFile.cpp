
#include "NpkFile.h"
#include <cstdio>
#include <filesystem>
#include "PvfString.h"


const char* HeaderFlag = "NeoplePack_Bill";


NpkFile::NpkFile(const std::string& initFile)
	:fileName(initFile)
{

}
auto NpkFile::openFile() -> void
{
	file = fopen(fileName.c_str(), "rb");
	if (file == nullptr) {
		printf("fail to open this file : %s", fileName.c_str());
		return;
	}
	fseek(file, 0, SEEK_END);
	length = ftell(file);
	fseek(file, 0, SEEK_SET);
}

auto NpkFile::loadAll(const std::string& path) -> void
{
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		bool isDir = std::filesystem::is_directory(entry);

		if (
			PvfString::endWith(entry.path().string(), ".npk") ||
			PvfString::endWith(entry.path().string(), ".NPK")
			&& !isDir) 
		{
			std::string path = entry.path().string();
		//	PvfString::toLower(path);

			GlobalFileTable.emplace(
				path,
				path
			);// .first->second.unpack();
		}
	}
}

auto NpkFile::unpack() -> void
{
	openFile();
	NpkHeader header;
	readBytes(reinterpret_cast<uint8_t*>(&header), sizeof(NpkHeader));

	if (strcmp(header.flag, HeaderFlag) != 0) {
		printf("is not a valid npk file");
		return;
	}

	for (int32_t i = 0; i < header.count; ++i)
	{
		auto & node = imgNodes.emplace_back(this);
		node.unpack();
	}

	for (auto & node : imgNodes)
	{
		GlobalTable[node.getFileName()] = &node;
		node.expand();
	}
}

auto NpkFile::setPosition(uint32_t position) -> void
{
	if (position > length)
	{
		printf("NpkFile :: OutOfFileSizeException : %d \n", position);
		return;
	}
	fseek(file, position, SEEK_SET);
	this->offset = position;
}

auto NpkFile::readBytes(uint32_t length) ->std::unique_ptr<uint8_t[]>
{
	auto bytes = std::make_unique<uint8_t[]>(length);
	fread(bytes.get(), length, 1, file);
	offset += length;
	return bytes;
}

auto NpkFile::readBytes(uint8_t* data, int32_t len) ->void
{
	fread(data, len, 1, file);
	offset += len;
}

auto NpkFile::readString(int32_t len) -> std::string
{
	std::string str;
	str.resize(len);
	fread(str.data(), len, 1, file);
	offset += len;
	return str;
}


auto NpkFile::expand(const std::string& name) -> void
{
	
}

#ifdef _WIN32
static const std::string delimiter = "\\";
#else
static const std::string delimiter = "/";
#endif

std::unordered_map<std::string, ImgFile*> NpkFile::GlobalTable;

std::unordered_map<std::string, NpkFile> NpkFile::GlobalFileTable;

auto NpkFile::getNpkImgNode(const std::string& path, int32_t index) -> ImgNode&
{
	std::vector<std::string> outs;
	PvfString::split(path, "/", outs);
	std::string newStr = "ImagePacks2"+ delimiter +"sprite_";
	std::string newStr2 = "sprite/";

	for (auto i = 0;i<outs.size() - 1;i++)
	{
		newStr.append(outs[i]);
		newStr.append("_");
		newStr2.append(outs[i]);
		newStr2.append("/");
	}
	newStr = newStr.substr(0, newStr.size() - 1);

	newStr.append(".NPK");

	newStr2.append(outs[outs.size() - 1]);
	auto node = GlobalTable[newStr2];
	if (node == nullptr) {
		GlobalFileTable.emplace(newStr, newStr).first->second.unpack();
	}
	return (*GlobalTable[newStr2])[index];
}
