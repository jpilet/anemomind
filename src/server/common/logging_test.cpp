/*
\author Julien Pilet <julien.pilet@gmail.com>
\date 2012

Tests for logging.c/cpp
*/


#include "logging.h"
#include "gtest/gtest.h"
#include <string.h>
#include <stdio.h>

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace sail;

//custom log handler that prints logging message with header in the console
void CustomLogHandler(LogLevel level, const char* filename, int line,
                      const std::string& message) {
  static const char* level_names[] = { "INFO", "WARNING", "ERROR", "FATAL" };

  int length = strlen(filename) + message.size() + 256;
  char* str = new char[length];
  snprintf(str, length, "[%s %s:%d] %s\n",
           level_names[level], filename, line, message.c_str());
  str[length - 1] = 0;

  printf("%s",str);

  delete[] str;
}

TEST(LoggingTest, MainTest) {
  LOG(INFO) << "Default Logging test.";

  int a = 1;
  long b = 2;
  float c = 3.0f;
  float d = 4.0;
  std::string s = "test";
  LOG(WARNING) << "Warning test, with variable printing: "
               << a << ", "
               << b << ", "
               << c << ", "
               << d << ", "
               << s << ".";

  LOG(ERROR) << "Default Error test";


  SetLogHandler(&CustomLogHandler);

  LOG(INFO) << "Custom Logging test:";
  LOG(WARNING) << "Custom Warning test, with variable printing: "
               << a << ", "
               << b << ", "
               << c << ", "
               << d << ", "
               << s << ".";

  EXPECT_NO_THROW(CHECK_NOTNULL(&a));
  EXPECT_THROW(CHECK_NOTNULL(nullptr), internal::LogMessageException);
}

struct TestStruct { };
std::ostream& operator<<(std::ostream& stream, TestStruct a) {
  return stream << "TestStruct";
}

TEST(LoggingTest, StreamableTest) {
  LOG(INFO) << TestStruct();
}
