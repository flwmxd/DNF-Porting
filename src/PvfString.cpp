
#include <ctime>
#include <stdio.h>
#include <string>
#include <memory.h>
#include <algorithm>
#include "PvfString.h"


auto PvfString::startWith(const std::string& str, const std::string& start) -> bool
{
	return str.compare(0, start.size(), start) == 0;
}

auto PvfString::contains(const std::string& str, const std::string& start) -> bool
{
	return str.find(start) != std::string::npos;
}

auto PvfString::endWith(const std::string& str, const std::string& start) -> bool
{
	return str.compare(str.length() - start.length(), start.size(), start) == 0;
}

auto PvfString::split(std::string input, const std::string& delimiter, std::vector<std::string>& outs) -> void
{
	size_t pos = 0;
	std::string token;
	while ((pos = input.find(delimiter)) != std::string::npos)
	{
		token = input.substr(0, pos);
		outs.push_back(token);
		input.erase(0, pos + delimiter.length());
	}
	outs.push_back(input);
}


auto PvfString::trim(std::string& str, const std::string& trimStr) -> void
{
	if (!str.empty())
	{
		str.erase(0, str.find_first_not_of(trimStr));
		str.erase(str.find_last_not_of(trimStr) + 1);
	}
}

auto PvfString::toLower(std::string& data) -> void
{
	std::transform(data.begin(), data.end(), data.begin(),
		[](unsigned char c) { return std::tolower(c); });
}

