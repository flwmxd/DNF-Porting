#include "PvfReader.h"
#include "iconv.h"
#include <iostream>
#include <filesystem>
#include <cassert>
#include <tellenc.h>
#include <set>

#include "PvfString.h"

inline static auto rotateRight4(uint32_t x, uint32_t y) -> uint32_t {
	return x >> y | x << 32 - y;
}

// trim from start (in place)
static inline auto ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
		}));
}

// trim from end (in place)
static inline auto rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

// trim from both ends (in place)
static inline auto trim(std::string& s) {
	ltrim(s);
	rtrim(s);
}


static inline auto split(const std::string& line, const std::string& a, const std::string& b) -> std::string
{
	auto index = 0;
	if (a != "")
		index = line.find_first_of(a);//如果a不为空在行中搜索a
	if (index == std::string::npos)
		return "";
	index = index + a.length();
	auto str = line.substr(index, line.length() - index);//对line提取a后面的部分
	if (b == "")
		return str;
	auto num = str.find_first_of(b);
	if (num == std::string::npos)
	{
		return "";
	}
	return str.substr(0, num);
}

static inline auto toLower(std::string& data) {
	std::transform(data.begin(), data.end(), data.begin(),
		[](unsigned char c) { return std::tolower(c); });
}

PvfReader::PvfReader(const std::string& path)
{
	file = fopen(path.c_str(), "rb");
	if (file == nullptr) {
		printf("fail to open this file : %s", path.c_str());
		exit(0);
	}
	fseek(file, 0, SEEK_END);
	length = ftell(file);
	fseek(file, 0, SEEK_SET);
}

PvfReader::~PvfReader()
{
	
	if (file != nullptr)
		fclose(file);
}

auto PvfReader::codeConvert(const char* fromCharset, const char* toCharset, const char* inbuf, size_t inlen,
	char* outbuf, size_t outlen) -> int32_t
{
	iconv_t cd;
	const char* temp = inbuf;
	char** pout = &outbuf;
	memset(outbuf, 0, outlen);
	cd = iconv_open(toCharset, fromCharset);
	if (cd == nullptr)
		return -1;

	iconv(cd, const_cast<char**>(&inbuf), &inlen, pout, &outlen);
	iconv_close(cd);
	return outlen;
}


auto PvfReader::unpack() -> void
{
	fread(&header, sizeof(PvfHeader), 1, file);
	auto headLength = header.dirTreeLength;//获取文件索引列表字节总大小
	auto dirTreeData = new uint8_t[header.dirTreeLength];
	fread(dirTreeData, header.dirTreeLength, 1, file);
	decrypt(dirTreeData, header.dirTreeLength, header.dirTreeChecksum);

	int32_t offset = 0;
	auto outChr = new char[1024];

	for (int32_t i = 0; i < header.numFilesInDirTree; i++)
	{

		auto fileNumber		= read<uint32_t>(dirTreeData, offset);
		auto filePathLength = read<int32_t>(dirTreeData, offset + 4);
		auto filePath		= dirTreeData + offset + 8;
		auto fileLength		= read<int32_t>(dirTreeData, offset + 8 + filePathLength);
		auto fileCrc32		= read<uint32_t>(dirTreeData, offset + 12 + filePathLength);
		auto relativeOffset = read<int32_t>(dirTreeData, offset + 0x10 + filePathLength);

		//CP949(韩语)
		codeConvert(PvfReader::ENCODING, "UTF-8", (char*)filePath, filePathLength, outChr, filePathLength * 2);
		std::string filePathName(outChr);
		rtrim(filePathName);
		auto & node			= pvfNodes[filePathName];
		node.fileNumber		= fileNumber;
		node.filePathLength = filePathLength;
		node.offset			= filePath;
		node.fileLength		= fileLength;
		node.fileCrc32		= fileCrc32;
		node.relativeOffset = sizeof(PvfHeader) + header.dirTreeLength + relativeOffset;
		node.reader			= this;
		node.fileName		= filePathName;
		offset += filePathLength + 20;
	}
	unpackStringTable(nullptr,nullptr);
	mapping();

	delete[] outChr;
	delete[] dirTreeData;
}

auto PvfReader::setPosition(uint64_t position) -> void
{
	if (position >= length) 
	{
		printf("PvfReader :: OutOfFileSizeException : %lld \n",position);
		return;
	}
	fseek(file, position, SEEK_SET);
	this->pos = position;
}


auto PvfReader::decrypt(uint8_t* ptr, uint32_t len, uint32_t crc32) -> void
{
	uint32_t* newPtr = reinterpret_cast<uint32_t*>(ptr);
	int32_t i = 0;
	while ( i < len / 4)
	{
		newPtr[i] = rotateRight4(newPtr[i] ^ PASSWORD_PVF ^ crc32, 6);
		i++;
	}

/*
	int32_t index = 0;
	while (index < len )
	{ 
		auto intv = read<uint32_t>(ptr, index);
		auto decryptWord = rotateRight4(intv ^ PASSWORD_PVF ^ crc32, 6);
		memcpy(ptr + index, decryptWord, 4);
		index += 4;
	}*/
}

auto PvfReader::dfsCreateNode(PvfNode& tag, PvfTreeNode * tree, const std::vector<std::string>& pathes, int32_t deep) -> void
{
	if (pathes.size() - 1 == deep)
	{
		tree->children[pathes[deep]] = std::make_unique<PvfTreeNode>(pathes[deep]);
		tree->children[pathes[deep]]->parent = tree;
		tree->children[pathes[deep]]->node = &tag;
		return;
	}

	if (tree->children.find(pathes[deep]) == tree->children.end())
	{
		tree->children[pathes[deep]] = std::make_unique<PvfTreeNode>(pathes[deep]);
		tree->children[pathes[deep]]->parent = tree;
	}
	auto & item1 = tree->children[pathes[deep]];
	dfsCreateNode(tag, item1.get(), pathes, deep + 1);
}

auto PvfReader::mapping() -> void
{
	for (auto & kv : pvfNodes)
	{
		std::vector<std::string> out;
		PvfString::split(kv.first,"/", out);
		dfsCreateNode(kv.second, &root, out, 0);
	}
	std::cout << "mapping over"<<std::endl;
	loaded = true;
} 

auto PvfReader::unpackStringTable(const std::function<void(std::function<void* ()>, std::function<void(void*)>,int32_t)>& addTask, const std::function<void()>& waitAll) -> void
{
	auto& strtable = pvfNodes["stringtable.bin"];

	auto ptr = strtable.expand();
	
	auto buffer = ptr.get();

	int32_t count = read<int32_t>(ptr.get(), 0);

	stringBinMap.resize(count);
	char* outChars = new char[32767];
	for (int32_t i = 0; i < count; i++)
	{
		auto startPos = read<int32_t>(buffer, i * 4 + 4);//每次循环的第一个int是键开始的地址
		auto endPos = read<int32_t>	 (buffer, i * 4 + 8);//每次循环的第二个int是键结束的地址
		auto len = endPos - startPos;//相减就是值的长度
		int32_t index = i;//索引就是出现的第几个

	
		codeConvert(PvfReader::ENCODING, "UTF-8", (char*)buffer + startPos + 4, len, outChars, len * 2);
		this->stringBinMap[index] = { outChars };//放到索引表中备用
		toLower(this->stringBinMap[index]);
		trim(this->stringBinMap[index]);
	
	}
	delete[] outChars;
	//##################################
	auto& nstrtable = pvfNodes["n_string.lst"];
	ptr = nstrtable.expand();
	auto len = nstrtable.getComputedFileLength();

	auto magicNumber = read<uint16_t>(ptr.get(), 0);
	assert(magicNumber == 53424);

	for (auto i = 2; i < len; i += 10)
	{
		if (len - i >= 10)//如果是最后十个字节或者最后不满十个字节就不执行
		{
			//前6位干嘛的不知道，6-10位的int值是stringtable的键，取出来
			auto v = read<int32_t>(ptr.get(), i + 6);
			const auto & k = stringBinMap[v];
			//取出来的stringtable的值是文件列表的一个文件的文件名，不过使用了驼峰命名需要将其置为小写并清除空格。

			if (auto node = pvfNodes.find(k); node != pvfNodes.end())
			{
				auto full = std::static_pointer_cast<PvfTextScript>(node->second.unpack());
				std::vector<std::string> out;
				PvfString::split(full->getContent(),"\r\n", out);

				for (auto& line : out)//根据换行分割，逐行遍历
				{
					if (auto pos = line.find_first_of('>'); pos != line.npos)//行包含符号'>'，如name_xxx>格斗家
					{
						auto key = split(line, "", ">");
						auto val = split(line, ">", "");
						stringStringMap[key] = val;//放到索引表中备用
					}
				}
			}
		}
	}
}

auto PvfReader::write(const std::string& file, const std::string& str) -> void
{
	std::string delimiter = "/";
	std::vector<std::string> outs;
	PvfString::split ("/", file, outs);

	if (outs.size() > 1) {
		auto pos = file.find_last_of(delimiter);
		if (pos != std::string::npos) {
			auto name = file.substr(0, pos);
			std::filesystem::create_directories(name);
		}
	}

	FILE* f = fopen(file.c_str(), "wb");
	fwrite(str.c_str(), str.length(), 1, f);
	fclose(f);
}

auto PvfReader::operator[](const std::string& path) ->PvfTreeNode&
{
	return root[path];
}

auto PvfReader::readBytes(uint32_t length) ->std::unique_ptr<uint8_t[]>
{
	auto bytes = std::make_unique<uint8_t[]>(length);
	fread(bytes.get(), length, 1, file);
	return bytes;
}


auto PvfReader::decryptString(const std::unique_ptr<uint8_t[]>& buffer, int32_t len, std::string& out) -> void
{
	/*if (len > 7) {
		for (int32_t i = 2; i < len; i += 5)//以5为单步从第二位开始遍历字节
		{
			if (len - i >= 5)//到最后了就不处理了防止内存越界
			{
				auto type = buffer.get()[i];//猜测应该是内容指示位
				if (type>=1 && type <= 10)
				{
					auto index = read<int32_t>(buffer.get(), i + 1);

					switch (type) {
					case ValueType::Value1:
					case ValueType::Value3:
					case ValueType::Value9:
					case ValueType::Int://2
					{
						auto value = std::to_string(index) +'\t';
						out.append(value);
					}
					break;
					case ValueType::Float://4
					{
						float f = *reinterpret_cast<float*>(&index);
						auto value = std::to_string(f) + '\t';
						out.append(value);
					}
					break;
			
					case ValueType::IntString5:
					{
						out.append("\r\n");
						out.append(stringBinMap[index]);
						out.append("\r\n");
					}
						break;
					case ValueType::IntString7:
					{
						out.append("`");
						out.append(stringBinMap[index]);
						out.append("`\r\n");
					}
						break;
					case ValueType::IntString6:
					case ValueType::IntString8: 
					{
						out.append("{");
						out.append(std::to_string(type));
						out.append("=`");
						out.append(stringBinMap[index]);
						out.append("`}\r\n");
					}
						break;
				
					case ValueType::StringTable:
					{
						auto before = read<int32_t>(buffer.get(), i  - 4);

						if (auto str = stringBinMap[index];str != "") {
							out.append("<");
							out.append(std::to_string(before));
							out.append("::");
							out.append(str);
							out.append("`");
							out.append(stringStringMap[str]);
							out.append("`>");
						}
						out.append("\r\n");
					}
						break;
					}
				}
				else 
				{
					std::cout << "Unknown type in pvf node ："<< type << std::endl;
				}
			}
		}
	}*/
}


