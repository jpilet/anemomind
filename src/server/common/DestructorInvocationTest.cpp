/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>

namespace {
  class MyStringClass {
   public:
    MyStringClass(char *stringData) : _stringData(stringData) {
      _stringData[0] = '1';
    }

    const char *c_str() const {
      return _stringData;
    }

    ~MyStringClass() {
      _stringData[0] = '0';
    }
   private:
    char *_stringData;
  };

  class MockObj {
   public:
    MockObj() {
      _stringData[0] = ' ';
      _stringData[1] = 0;
    }
    MyStringClass str() {
      return MyStringClass(_stringData);
    }
   private:
    char _stringData[2];
  };

  std::string result;

  void assignResult(const char *x) {
    result = x;
  }
}

TEST(DestructorInvocationTest, StringTest) {
  MockObj obj;
  assignResult(obj.str().c_str());
  EXPECT_TRUE(result == "1");
}


