
#include "PvfNode.h"
#include "PvfReader.h"
#include "PvfAnimation.h"
#include "PvfDocument.h"
#include "PvfString.h"
#include <assert.h>

auto PvfNode::unpack() -> std::shared_ptr<PvfScript>
{
	if (pvfScript != nullptr) 
	{
		return pvfScript;
	}
	auto buffer = expand();
	if (PvfString::endWith(fileName,".ani"))
	{
		pvfScript = std::make_shared<PvfAnimation>(buffer.get(), getComputedFileLength(),reader);
	}
	else if (PvfString::endWith(fileName, ".str"))
	{
		pvfScript = std::make_shared<PvfTextScript>(buffer.get(), getComputedFileLength(), reader);
	}
	else
	{
		pvfScript = std::make_shared<PvfDocument>(buffer.get(), getComputedFileLength(), reader);
	}
	pvfScript->unpack();
	return pvfScript;
}

auto PvfNode::expand() -> std::unique_ptr<uint8_t[]>
{
	if (fileLength > 0) {
		auto computedFileLength = (int32_t)((fileLength + 3L) & 4294967292L);
		reader->setPosition(relativeOffset);
		auto bytes = reader->readBytes(computedFileLength);
		reader->decrypt(bytes.get(), computedFileLength, fileCrc32);
	
		for (int32_t i = 0; i < (computedFileLength - fileLength); i++)
		{
			bytes.get()[fileLength + i] = 0;
		}
		return bytes;
	} 
	return nullptr;
}


auto PvfNode::getComputedFileLength() const -> int32_t
{
	return (int32_t)((fileLength + 3L) & 4294967292L);
}


PvfTreeNode PvfTreeNode::nullNode = {};

auto PvfTreeNode::operator[](const std::string& path) ->  PvfTreeNode&
{
	if (auto iter = children.find(path); iter != children.end()) {
		return *iter->second;
	}
	return PvfTreeNode::nullNode;
}

auto PvfTreeNode::getByPath(const std::string& path) ->  PvfTreeNode&
{
	std::vector<std::string> outs;
	PvfString::split(path, "/", outs);
	PvfTreeNode * node = this;
	for (auto & path : outs)
	{
		node = &(*node)[path];
	}
	return *node;
}

auto PvfTreeNode::unpack() ->std::shared_ptr< PvfScript>
{
	assert(node != nullptr); return node->unpack();
}
