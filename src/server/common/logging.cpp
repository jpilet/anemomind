// Julien Pilet <julien.pilet@gmail.com>
// 2012
//
// Logging functions where taken from Google's protocol buffers. It had the
// following notice:
//
// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN  // We only need minimal includes
#include <windows.h>
#define snprintf _snprintf
#endif

#include "logging.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>

using std::string;

namespace sail {

namespace internal {

LogMessage& LogMessage::operator<<(const string& value) {
  message_ += value;
  return *this;
}

LogMessage& LogMessage::operator<<(const char* value) {
  message_ += value;
  return *this;
}

#undef DECLARE_STREAM_OPERATOR
#define DECLARE_STREAM_OPERATOR(TYPE, FORMAT)                       \
  LogMessage& LogMessage::operator<<(TYPE value) {                  \
    /* 128 bytes should be big enough for any of the primitive */   \
    /* values which we print with this, but we use snprintf() */  \
    /* anyway to be extra safe. */                                  \
    char buffer[128];                                               \
   snprintf(buffer, sizeof(buffer), FORMAT, value);                \
    /* Guard against broken MSVC snprintf(). */                     \
    buffer[sizeof(buffer)-1] = '\0';                                \
    message_ += buffer;                                             \
    return *this;                                                   \
  }

DECLARE_STREAM_OPERATOR(char         , "%c" )
DECLARE_STREAM_OPERATOR(int          , "%d" )
DECLARE_STREAM_OPERATOR(unsigned int , "%u" )
DECLARE_STREAM_OPERATOR(long         , "%ld")
DECLARE_STREAM_OPERATOR(unsigned long, "%lu")
DECLARE_STREAM_OPERATOR(double       , "%g" )
DECLARE_STREAM_OPERATOR(long long    , "%lld" )
#undef DECLARE_STREAM_OPERATOR


#ifdef WIN32
void DefaultLogHandler(LogLevel level, const char* filename, int line,
                       const std::string& message) {
  static const char* level_names[] = { "INFO", "WARNING", "ERROR", "FATAL" };

  int length = strlen(filename) + message.size() + 256;
  char* str = new char[length];
  snprintf(str, length, "[%s %s:%d] %s\n",
           level_names[level], filename, line, message.c_str());
  str[length - 1] = 0;

  OutputDebugString(str);
  if ( (level == LOGLEVEL_ERROR) || (level == LOGLEVEL_FATAL) )
    MessageBox(NULL, str, level_names[level], MB_OK);
  delete[] str;
}
#else
void DefaultLogHandler(LogLevel level, const char* filename, int line,
                       const std::string& message) {
  static const char* level_names[] = { "INFO", "WARNING", "ERROR", "FATAL" };

  // We use fprintf() instead of cerr because we want this to work at static
  // initialization time.
  fprintf(stderr, "[%s %s:%d] %s\n",
          level_names[level], filename, line, message.c_str());
}
#endif

void (*LogHandler)(LogLevel level, const char* filename, int line,
                   const std::string& message) = DefaultLogHandler;
LogLevel LogLevelThreshold = LOGLEVEL_INFO;



LogMessage::LogMessage(LogLevel level, const char* filename, int line)
  : level_(level), filename_(filename), line_(line)  {}
LogMessage::~LogMessage() { }

void LogMessage::Finish() {
  if(level_ >= LogLevelThreshold) {
    LogHandler(level_, filename_, line_, message_);
  }

  if (level_ == LOGLEVEL_FATAL) {
    throw LogMessageException(level_, filename_, line_, message_);
  }
}

LogMessageException::LogMessageException(LogLevel level, const char* filename, int line, const std::string &message) :
  level_(level), filename_(filename), line_(line), message_(message) {}

void LogFinisher::operator=(LogMessage& other) {
  other.Finish();
}
}  // namespace internal

void SetLogHandler(void (*log_handler)(LogLevel level, const char* filename, int line,
                                       const std::string& message)) {
  internal::LogHandler = log_handler;
}

void SetLogLevelThreshold(LogLevel level) {
  internal::LogLevelThreshold = level;
}

}  // namespace sail
