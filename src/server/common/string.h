#ifndef TEXT_H_
#define TEXT_H_

#include <string>
#include <sstream>
#include <server/common/Array.h>
#include <server/common/Optional.h>
#include <iostream>

namespace sail {

template <typename T>
std::string objectToString(const T &x) {
  std::stringstream ss;
  ss << x;
  return ss.str();
}

#define EXPR_AND_VAL_AS_STRING(X) (std::string(#X " = \n") + objectToString((X)))

bool tryParseInt(const std::string &s, int *out);
bool tryParseDouble(const std::string &s, double *out);

template <typename T>
Optional<T> tryParse(const std::string& src) {
  std::stringstream ss;
  ss << src;
  T dst;
  if (ss >> dst) {
    return dst;
  }
  return {};
}

template <typename T>
struct PrimitiveParser {
  Optional<T> operator()(const std::string& s) const {
    return tryParse<T>(s);
  }
};

bool isEscaped(char c);
unsigned char decodeHexDigit(char c);
bool isHexDigit(char c);

// 0x-prefix not supported
bool areHexDigits(int count, const char *c);
bool isHexString(const std::string &s, int expectedLength = -1);

std::string getEscapeString(char c);
char toHexDigit(int value);
std::string bytesToHex(size_t n, uint8_t *bytes);
std::string formatInt(const std::string &fstr, int value);
std::string stringFormat(const char *fmt, ...);
void toLowerInPlace(std::string &data);
std::string toLower(const std::string &src);
void splitFilenamePrefixSuffix(const std::string &filename,
                               std::string &prefix, std::string &suffix);
void indent(std::ostream *s, int count);

std::string readFileToString(const std::string& filename);
Array<std::string> split(std::string x, char delimiter);

bool isBlank(char c);
bool isBlankString(const std::string& s);

std::string sliceString(const std::string& s, int from, int to);

template <class StringCollection>
std::string join(const StringCollection& array, const std::string& delimiter) {
  std::string result;
  bool first = true;
  for (auto s : array) {
    if (first) {
      first = false;
    } else {
      result += delimiter;
    }
    result += s;
  }
  return result;
}

static bool startsWith(const std::string& s, const char *prefix) {
  const int end = s.size();
  for (int i = 0; prefix[i] != 0; ++i) {
    if (i == end || prefix[i] != s[i]) {
      return false;
    }
  }
  return true;
}

} /* namespace sail */
#endif /* TEXT_H_ */
