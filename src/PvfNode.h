#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>
#include "PvfScript.h"


class PvfNode;
class PvfAnimation;
class PvfDocument;

struct PvfTreeNode
{
	static PvfTreeNode nullNode;
	auto operator[](const std::string& path) -> PvfTreeNode&;
	auto getByPath(const std::string& path) -> PvfTreeNode&;
	auto unpack()->std::shared_ptr< PvfScript>;
	inline auto isValid() { return this != &nullNode; }

	PvfTreeNode(const std::string& name = "") :name(name) {  };
	std::string name;
	std::unordered_map<std::string, std::unique_ptr<PvfTreeNode>> children;
	PvfNode* node = nullptr;
	PvfTreeNode* parent = nullptr;
};

class PvfNode 
{
public:
	friend class PvfReader;
	auto unpack() ->std::shared_ptr<PvfScript>;
private:
	auto expand() -> std::unique_ptr<uint8_t[]>;
	auto getComputedFileLength() const->int32_t;
	PvfReader* reader = nullptr;
	uint32_t fileNumber = 0;
	int32_t filePathLength = 0;
	uint8_t * offset = nullptr;
	int32_t fileLength = 0;
	uint32_t fileCrc32 = 0;
	int32_t relativeOffset = 0;
	std::string fileName;
	std::string content;
	std::shared_ptr<PvfScript> pvfScript;
};