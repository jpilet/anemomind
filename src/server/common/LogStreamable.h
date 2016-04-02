/*
 * LogStreamable.h
 *
 *  Created on: Apr 2, 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_LOGSTREAMABLE_H_
#define SERVER_COMMON_LOGSTREAMABLE_H_

#include <sstream>
#include <server/common/logging.h>

// TODO in later PR: Would it be easy to make the logging API
// to log objects for which the '<<' operator is implemented with ostream?
template <typename T>
void logStreamable(LogLevel level, const T &x, const char *file, int line) {
  std::stringstream ss;
  ss << x;
  while (ss.good()) {
    std::string s;
    std::getline(ss, s);
    internal::LogFinisher() = internal::LogMessage(level, file, line) << s;
  }
}

#define LOG_STREAMABLE(level, x) \
    logStreamable(LOGLEVEL_##level, x, __FILE__, __LINE__)

#endif /* SERVER_COMMON_LOGSTREAMABLE_H_ */
