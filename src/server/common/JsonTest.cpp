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
    Array<T> arr = makeTestArray<T>(len, 256);
    Array<T> arr2;

    Poco::JSON::Array::Ptr testarr(new Poco::JSON::Array());

    json::deserialize(json::serialize(arr), &arr2);
    for (int i = 0; i < len; i++) {
      EXPECT_EQ(arr[i], arr2[i]);
    }
  }
}


TEST(JsonTest, UIntArray) {primitiveArrayTest<unsigned int>();}
TEST(JsonTest, IntArray) {primitiveArrayTest<int>();}
TEST(JsonTest, DoubleArray) {primitiveArrayTest<double>();}
TEST(JsonTest, FloatArray) {primitiveArrayTest<float>();}
TEST(JsonTest, BoolArray) {primitiveArrayTest<bool>();}
TEST(JsonTest, UCharArray) {primitiveArrayTest<unsigned char>();}
TEST(JsonTest, CharArray) {primitiveArrayTest<char>();}
TEST(JsonTest, ULongArray) {primitiveArrayTest<unsigned long int>();}
TEST(JsonTest, LongArray) {primitiveArrayTest<long int>();}
TEST(JsonTest, UShortArray) {primitiveArrayTest<unsigned short int>();}
TEST(JsonTest, ShortArray) {primitiveArrayTest<short int>();}




