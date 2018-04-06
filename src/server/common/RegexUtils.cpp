/*
 * RegexUtils.cpp
 *
 *  Created on: 6 Apr 2018
 *      Author: jonas
 */

#include "RegexUtils.h"

#include <string>
#include <regex>

namespace sail {
namespace Regex {

std::string captureGroup(const std::string& s) {
  return "(" + s + ")";
}

std::string nonCaptureGroup(const std::string& s) {
  return "(?:" + s + ")";
}

// Use this instead of '+' to concatenate patterns, so that the
// evaluation order is preserved.
std::string operator/(const std::string& a, const std::string& b) {
  return nonCaptureGroup(a + b);
}

std::string operator|(const std::string& a, const std::string& b) {
  return nonCaptureGroup(a + "|" + b);
}

std::string anyCount(const std::string& s) {
  return s/"*";
}

std::string atLeastOnce(const std::string& s) {
  return s/"+";
}

std::string entireString(const std::string& s) {
  return "^"/s/"$";
}

std::string maybe(const std::string& s) {
  return s/"?";
}

std::string space = "[ \\t\\r\\n\\v\\f]"; // "[:space:]" doesn't work;
std::string digit = "[0-9]";
std::string xdigit = "[A-Fa-f0-9]";

std::string unsignedInteger(const std::string& d) {
  return atLeastOnce(d);
}

std::string maybeSign = maybe("[+-]");

// Helper for join
std::string moreItems(
    const std::string& separatorPattern,
    const std::string& itemPattern) {
  return anyCount(separatorPattern/itemPattern);
}

std::string join1(
    const std::string& separatorPattern,
    const std::string& itemPattern) {
  return itemPattern/moreItems(separatorPattern, itemPattern);
}

std::string join0(
    const std::string& separatorPattern,
    const std::string& itemPattern) {
  return maybe(join1(separatorPattern, itemPattern));
}

std::string signedNumber(const std::string& num) {
  return maybeSign/num;
}

std::string fractionalSeparator = "\\.";

std::string unsignedFractionalNumber(const std::string& d) {
  auto digits = unsignedInteger(d);
  return (digits/fractionalSeparator/digits)
      | (digits/fractionalSeparator)
      | (fractionalSeparator/digits);
}

std::string basicNumber(const std::string& d) {
  return signedNumber(unsignedInteger(d) | unsignedFractionalNumber(d));
}


}
} /* namespace sail */
