
#pragma once
#include <string>
#include <vector>
#include <stdarg.h>
#include <functional>
#include <memory>

namespace PvfString
{

	auto split(std::string input, const std::string& delimiter, std::vector<std::string>& outs) -> void;
	auto startWith(const std::string& str, const std::string& start) -> bool;
	auto contains(const std::string& str, const std::string& start) -> bool;
	auto endWith(const std::string& str, const std::string& start) -> bool;
	auto trim(std::string& str,const std::string & trimStr = " ") -> void;
	auto toLower(std::string& data) -> void;
#ifdef _WIN32
	static const std::string delimiter = "\\";
#else
	static const std::string delimiter = "/";
#endif
}; 
