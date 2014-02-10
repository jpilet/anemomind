#ifndef TEXT_H_
#define TEXT_H_

#include <string>


namespace sail
{

bool notIsBlank(char c);
bool isBlank(char c);

bool tryParseInt(std::string s, int &out);
bool tryParseDouble(std::string s, double &out);

bool isEscaped(char c);
std::string getEscapeString(char c);
char toHexDigit(int value);
std::string bytesToHex(size_t n, uint8_t *bytes);
std::string formatInt(std::string fstr, int value);
std::string stringFormat(const std::string fmt, ...);
void toLowerInPlace(std::string &data);
void splitFilenamePrefixSuffix(std::string filename,
		std::string &prefix, std::string &suffix);

} /* namespace sail */
#endif /* TEXT_H_ */
