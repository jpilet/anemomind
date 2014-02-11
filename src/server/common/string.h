#ifndef TEXT_H_
#define TEXT_H_

#include <string>
#include <sstream>


namespace sail {

template <typename T>
std::string objectToString(const T &x) {
  std::stringstream ss;
  ss << x;
  return ss.str();
}

#define EXPR_AND_VAL_AS_STRING(X) (std::string(#X " = \n") + objectToString(X))
#define EAVAS EXPR_AND_VAL_AS_STRING

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
