/*
 * RegexUtils.h
 *
 *  Created on: 6 Apr 2018
 *      Author: jonas
 */

#ifndef SERVER_COMMON_REGEXUTILS_H_
#define SERVER_COMMON_REGEXUTILS_H_

#include <string>
#include <regex>

namespace sail {
namespace Regex {


std::string captureGroup(const std::string& s);
std::string nonCaptureGroup(const std::string& s);

// Use this instead of '+' to concatenate patterns, so that the
// evaluation order is preserved.
std::string operator/(const std::string& a, const std::string& b);


std::string operator|(const std::string& a, const std::string& b);
std::string anyCount(const std::string& s);
std::string atLeastOnce(const std::string& s);
std::string entireString(const std::string& s);
std::string maybe(const std::string& s);
std::string charInString(const std::string& chars);
std::string charNotInString(const std::string& chars);
extern std::string spaceChars;
extern std::string space;
extern std::string nonspace;
extern std::string digit;
extern std::string xdigit;
std::string unsignedInteger(const std::string& d = digit);
extern std::string maybeSign;
std::string join1(
    const std::string& separatorPattern,
    const std::string& itemPattern);
std::string join0(
    const std::string& separatorPattern,
    const std::string& itemPattern);
std::string signedNumber(const std::string& num);
extern std::string fractionalSeparator;
std::string unsignedFractionalNumber(const std::string& digitPattern);
std::string basicNumber(const std::string& digit);

}
} /* namespace sail */

#endif /* SERVER_COMMON_REGEXUTILS_H_ */
