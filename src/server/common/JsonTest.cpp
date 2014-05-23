/*
 *  Created on: May 23, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/Json.h>
#include <gtest/gtest.h>
#include <limits>


using namespace sail;

namespace {
  template <typename T>
  Array<T> makeTestArray(int len, int limit) {
    Array<T> dst(len);
    for (int i = 0; i < len; i++) {
      dst[i] = (7*i) % limit;
    }
    return dst;
  }

  template <typename T>
  void primitiveArrayTest() {
    int len = 30;
    Array<T> arr = makeTestArray<T>(len, std::numeric_limits<T>::max());
    Array<T> arr2;

    Poco::JSON::Array::Ptr testarr(new Poco::JSON::Array());

    json::deserialize(json::serialize(arr), &arr2);
    for (int i = 0; i < len; i++) {
      EXPECT_EQ(arr[i], arr2[i]);
    }
  }
}

#define PRIMITIVE_ARRAY_TEST(name, type) TEST(JsonTest, name) {primitiveArrayTest<type>();}

PRIMITIVE_ARRAY_TEST(IntArray, int)
PRIMITIVE_ARRAY_TEST(DoubleArray, double)


