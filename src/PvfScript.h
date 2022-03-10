#pragma once
#include <cstdint>
#include <string>

class PvfReader;

enum PvfScriptType
{
	Animation,
	Text,
	Document
};

class PvfScript 
{
public:
	virtual auto unpack() -> void = 0;
    inline auto& getType() const { return type; }
protected:
	PvfScriptType type;

};


class PvfTextScript : public PvfScript
{
public:
	PvfTextScript(const uint8_t* buffer, int32_t len, PvfReader* reader);
	auto unpack() -> void override;
	inline auto& getContent() const { return str; }
private:
	const uint8_t* buffer = nullptr;
	PvfReader* reader = nullptr;
	int32_t len = 0;
	std::string str;
};
