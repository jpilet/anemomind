/*
 *  Created on: 31 janv. 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Array.h"
#include "gtest/gtest.h"


using namespace sail;

namespace {
  // Keeps track of how many instances are allocated in order
  // to detect memory leaks without valgrind.
  class MemoryTestObj {
    public:
      static int InstanceCounter;

      MemoryTestObj() {
        _dummyInt = InstanceCounter;
        InstanceCounter++;
      }

      ~MemoryTestObj() {
        InstanceCounter--;
      }

      int dummyInt() {return _dummyInt;}
    private:
      int _dummyInt; // Just here to make this object non-trivial.
  };

  int MemoryTestObj::InstanceCounter = 0;

  typedef Array<MemoryTestObj> TestArray;
}


TEST(ArrayTest, CopyTest) {
  {
    TestArray A(30);
    EXPECT_EQ(MemoryTestObj::InstanceCounter, 30);
    TestArray B = A;
    EXPECT_EQ(MemoryTestObj::InstanceCounter, 30);
  }
  EXPECT_EQ(MemoryTestObj::InstanceCounter, 0);
}

TEST(ArrayTest, SliceTest) {
  {
    TestArray A(30);
    EXPECT_EQ(MemoryTestObj::InstanceCounter, 30);
    TestArray B = A.sliceTo(15);
    EXPECT_EQ(MemoryTestObj::InstanceCounter, 30);
    TestArray C = A.sliceFrom(15);
    EXPECT_EQ(MemoryTestObj::InstanceCounter, 30);
  }
  EXPECT_EQ(MemoryTestObj::InstanceCounter, 0);
}

TEST(ArrayTest, SliceTestReplace) {
  {
    TestArray A(30);
    EXPECT_EQ(MemoryTestObj::InstanceCounter, 30);
    TestArray B = A.sliceTo(15);
    EXPECT_EQ(MemoryTestObj::InstanceCounter, 30);
    EXPECT_EQ(B.size(), 15);
    TestArray C = A.sliceFrom(15);
    EXPECT_EQ(MemoryTestObj::InstanceCounter, 30);
    EXPECT_EQ(C.size(), 15);
    EXPECT_EQ(MemoryTestObj::InstanceCounter, 30);
    B = TestArray();
    EXPECT_EQ(B.size(), 0);
    EXPECT_EQ(MemoryTestObj::InstanceCounter, 30);
  }
  EXPECT_EQ(MemoryTestObj::InstanceCounter, 0);
}
