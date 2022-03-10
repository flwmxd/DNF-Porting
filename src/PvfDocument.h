
#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <stack>
#include "PvfScript.h"

class PvfReader;

class PvfDocument : public PvfScript
{
public:

	enum AttributeType 
	{
		Number,
		String,
	};

	union DNumber
	{
		int32_t intValue;
		float floatValue;
	};

	struct IAttribute {
		AttributeType type;
	};

	template<typename T, AttributeType TType>
	struct Attribute : IAttribute
	{
		inline Attribute() : value() { type = TType; }
		T value;
	};

	struct NumberAttribute	final : public Attribute<DNumber, AttributeType::Number> {};
	struct StringAttribute	final : public Attribute<std::string, AttributeType::String> {};

	struct Node
	{
		std::string name;
		std::vector<std::shared_ptr<IAttribute>> attribute;
		std::unordered_map<
			std::string,
			std::vector<Node>
		> children;

		inline auto size() const { return attribute.size(); };

		inline auto& operator[](const std::string& name) { return children[name]; };

		template<typename T>
		inline auto to(int32_t index) const -> const T&{ 
			return T();
		}
		template<>
		inline auto to<float>(int32_t index) const -> const float& {
			return std::static_pointer_cast<NumberAttribute>(attribute[index])->value.floatValue;
		}
		template<>
		inline auto to<int32_t>(int32_t index) const -> const int32_t& {
			return std::static_pointer_cast<NumberAttribute>(attribute[index])->value.intValue;
		}
		template<>
		inline auto to<std::string>(int32_t index) const -> const std::string& {
			return std::static_pointer_cast<StringAttribute>(attribute[index])->value;
		}



		template<typename T>
		auto addAttribute(const T& t)->void {};

		template<>
		auto addAttribute(const float & t)->void 
		{
			auto att = std::make_shared<NumberAttribute>();
			att->value.floatValue = t;
			attribute.emplace_back(att);
		};

		template<>
		auto addAttribute(const int32_t & t)->void 
		{
			auto att = std::make_shared<NumberAttribute>();
			att->value.intValue = t;
			attribute.emplace_back(att);
		};

		template<>
		auto addAttribute(const std::string& t)->void 
		{
			auto att = std::make_shared<StringAttribute>();
			att->value = t;
			attribute.emplace_back(att);
		};
	
		bool hasEndTag = false;

	};

	PvfDocument(const uint8_t* buffer, int32_t len, PvfReader* reader);
	auto unpack() -> void override;

	inline auto& operator[](const std::string& name) {
		return root[name];
	}

	template<typename T>
	inline auto get(const std::string& name) -> const T& {
		return T();
	}

	template<>
	inline auto get<std::string>(const std::string& name)-> const std::string& 
	{
		static std::string nullName = "";
		auto attr = splitNode(name);
		return attr ? std::static_pointer_cast<StringAttribute>(attr)->value : nullName;
	}

	template<>
	inline auto get<float>(const std::string& name)->const float&
	{
		static float nullName = 0;
		auto attr = splitNode(name);
		return attr ? std::static_pointer_cast<NumberAttribute>(attr)->value.floatValue : nullName;
	}

	template<>
	inline auto get<int32_t>(const std::string& name)->const int32_t&
	{
		static int32_t nullName = 0;
		auto attr = splitNode(name);
		return attr ? std::static_pointer_cast<NumberAttribute>(attr)->value.intValue : nullName;
	}

private:
	
	auto splitNode(const std::string& name)->std::shared_ptr<IAttribute>;

	auto pop(std::stack<Node*>& stack, const std::string& name) -> void;

	const uint8_t* buffer;
	int32_t len;
	PvfReader* pvfReader = nullptr;
	Node root;
	Node* node = nullptr;

	static Node nullNode;

};