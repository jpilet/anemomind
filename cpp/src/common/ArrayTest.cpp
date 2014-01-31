/*
 *  Created on: 31 janv. 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Array.h"
#include "gtest/gtest.h"
#include "common.h"


using namespace sail;

namespace {
  // Keeps track of how many instances are allocated in order
  // to detect memory leaks without valgrind.
  class MemoryTestObj {
    public:
      static int InstanceCounter;

      MemoryTestObj() {
        _dummyInt = 119 + (InstanceCounter % 2); // Make it a bit non-trivial
        InstanceCounter++;
      }

      ~MemoryTestObj() {
        if (!(_dummyInt == 119 || _dummyInt == 120)) {
          throw std::runtime_error("You are probably trying to deallocate memory that was never allocated and initialized");
        }
        _dummyInt = 0;
        InstanceCounter--;
      }

      int dummyInt() {return _dummyInt;}
    private:
      int _dummyInt;
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
    B = TestArray();
    EXPECT_EQ(B.size(), 0);
    EXPECT_EQ(MemoryTestObj::InstanceCounter, 30);
    A = TestArray();
    EXPECT_EQ(A.size(), 0);
    EXPECT_EQ(MemoryTestObj::InstanceCounter, 30);
  }
  EXPECT_EQ(MemoryTestObj::InstanceCounter, 0);
}

TEST(ArrayTest, CreateTest) {
  {
    TestArray A(30);
    EXPECT_EQ(MemoryTestObj::InstanceCounter, 30);
    A.create(3);
    EXPECT_EQ(MemoryTestObj::InstanceCounter, 3);
  }
  EXPECT_EQ(MemoryTestObj::InstanceCounter, 0);
}

TEST(ArrayTest, MakeEmptyTest) {
  TestArray A(30);
  EXPECT_EQ(MemoryTestObj::InstanceCounter, 30);
  A = TestArray();
  EXPECT_EQ(MemoryTestObj::InstanceCounter, 0);
}

TEST(ArrayTest, OverwriteTest) {
  TestArray A(30);
  EXPECT_EQ(MemoryTestObj::InstanceCounter, 30);
  TestArray B(7);
  EXPECT_EQ(MemoryTestObj::InstanceCounter, 37);
  A = B;
  DOUT(MemoryTestObj::InstanceCounter);
  EXPECT_EQ(MemoryTestObj::InstanceCounter, 7);
  A = TestArray();
  EXPECT_EQ(MemoryTestObj::InstanceCounter, 7);
  B = TestArray();
  EXPECT_EQ(MemoryTestObj::InstanceCounter, 0);
}
