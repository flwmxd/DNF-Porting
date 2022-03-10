#include "PvfDocument.h"
#include "PvfNode.h"
#include <iostream>
#include "PvfReader.h"
#include "ValueType.h"
#include <stack>
#include <unordered_set>
#include "PvfString.h"
#include "BufferReader.h"



PvfDocument::PvfDocument(const uint8_t* buffer, int32_t len,PvfReader * reader)
	:buffer(buffer),len(len),pvfReader(reader)
{
	type = PvfScriptType::Document;
}


auto PvfDocument::unpack() -> void
{
	if (len > 7) {
		BufferReader reader(buffer, len);
		auto header = reader.read<int16_t>();

		std::unordered_set<std::string> tags;

		while (reader.getOffset() < len - 4)
		{
			auto type = reader.read<int8_t>(); 
			if (type >= 2 && type <= 10) {
				auto index = reader.read<int32_t>();
				if (type == ValueType::Section) 
				{
					tags.emplace(pvfReader->stringBinMap[index]);
				}
			}
		}

		reader.setOffset(2);
		node = &root;
		std::stack<Node*> stack;
		stack.push(node);

		
		while (reader.getOffset() < len) 
		{
			auto type = reader.read<int8_t>(); //到最后了就不处理了防止内存越界

			if (type >= 2 && type <= 10)
			{
				auto index = reader.read<int32_t>();

				switch (type) {
				case ValueType::IntEx:
				case ValueType::Int://2
				{
					node->addAttribute(index);
				}
				break;
				case ValueType::Float://4
				{
					float f = *reinterpret_cast<float*>(&index);
					node->addAttribute(f);
				}
				break;

				case ValueType::Section:
				{
					auto name = pvfReader->stringBinMap[index];
					auto endTagName = name;
					endTagName.insert(endTagName.begin() + 1, '/');

					if(!PvfString::startWith(name,"[/")){//Start Node
						
						PvfString::trim(name, "[");
						PvfString::trim(name, "]");
						if (node != nullptr && !node->hasEndTag && node != &root) 
						{
							pop(stack, name);
						}
						if (node != nullptr) // && node->hasEndTag
						{//new node is a child in pre-node
							auto& newNode = node->children[name].emplace_back();
							newNode.name = name;
							newNode.hasEndTag = tags.count(endTagName);
							stack.push(&newNode);
							node = &newNode;
						}
					}
					else //end Node
					{
						PvfString::trim(name, "[/");
						PvfString::trim(name, "]");
						pop(stack, name);
					}
					
				}
				break;

				case ValueType::String://Child
				{
					node->addAttribute(pvfReader->stringBinMap[index]);
				}
				break;
				case ValueType::Command:
				case ValueType::CommandSeparator:
				{
					node->addAttribute(pvfReader->stringBinMap[index]);
				}
				break;

				case ValueType::StringLink:
					if (auto str = pvfReader->stringBinMap[index]; str != "") {
						node->addAttribute(pvfReader->stringStringMap[str]);
						//std::cout<<str<<"  "<<pvfReader->stringStringMap[str]<<std::endl;
					}
					break;
				/*case ValueType::StringLinkIndex:
				{
					//	out.append(std::to_string(before));//(buffer.get(), i - 4);
					if (auto str = pvfReader->stringBinMap[index]; str != "") {
						node->addAttribute(pvfReader->stringStringMap[str]);
					}
				}*/
				break;
				}
			}
			else
			{
				std::cout << "Unknown type in pvf node ：" << (int32_t)type << std::endl;
			}
		}
	}
}

auto PvfDocument::splitNode(const std::string& name) ->std::shared_ptr<IAttribute>
{
	std::vector<std::string> outs;
	PvfString::split(name, "/", outs);
	auto& pvfNode = root[outs[0]];
	Node * node = nullptr;
	int32_t i = 1;
	while (i < outs.size() - 1)//last 
	{
		node = &pvfNode[std::stoi(outs[i++])];
	}
	return node != nullptr ? node->attribute[std::stoi(outs[i])] : nullptr;
}

auto PvfDocument::pop(std::stack<Node*>& stack, const std::string& name) -> void
{
	if (node != nullptr && node != &root)
	{
		if (!stack.empty()) {
			stack.pop();
		}

		if (!stack.empty()) {
			node = stack.top();
		}
		else
		{
			node = nullptr;
		}
	
		if (node != nullptr && node->name == name)
		{
			pop(stack, node->name);
		}
	}
}

PvfDocument::Node PvfDocument::nullNode;
