#include "PvfScript.h"
#include "PvfReader.h"
#include "tellenc.h"

PvfTextScript::PvfTextScript(const uint8_t* buffer, int32_t len, PvfReader* reader)
	:buffer(buffer),len(len),reader(reader)
{
	type = PvfScriptType::Text;
}

auto PvfTextScript::unpack() -> void 
{
	//libIconv 在(BIG5字符集下) 转换'恒'的时候会中断
	//使用BIG5HKSCS香港增补字符集 
	char* outBuffer = new char[len * 2];
	memset(outBuffer, 0, len * 2);

	//auto charset = tellenc(buffer, len);
	/*if (/ *charset == std::string("big5") || * /charset == std::string("binary")) {
		charset = PvfReader::ENCODING;
	}
*/
	int32_t retLen = 0;
	if (buffer && len > 0)
	{
		retLen = reader->codeConvert(PvfReader::ENCODING, "UTF-8", (const char*)buffer, len, outBuffer, len * 2);
		str = {outBuffer};
	}
}
