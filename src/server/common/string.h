#ifndef TEXT_H_
#define TEXT_H_

#include <string>
#include <sstream>
#include <server/common/Array.h>


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
std::string int64ToHex(int64_t x);
void indent(std::ostream *s, int count);

std::string readFileToString(const std::string& filename);
Array<std::string> split(std::string x, char delimiter);

template <typename T>
struct PositiveAsString {
  static const int StorageSize = 3*sizeof(T); // One byte needs at most three decimal digits.
  char storage[StorageSize];
  int start = 0;
  const char* str() const {return storage + start;}

  PositiveAsString(T x) {
    start = StorageSize-1;
    storage[start] = '\0';
    T y = x;
    if (x == 0) {
      storage[--start] = '0';
    } else {
      while (y > 0) {
        storage[--start] = '0' + (y % 10);
        y = y/10;
      }
    }
  }
};

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
} /* namespace sail */
#endif /* TEXT_H_ */
