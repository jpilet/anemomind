/*
 *  Created on: 2014-07-01
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/VersionedEnum.h>
#include <gtest/gtest.h>

using namespace sail;

namespace {
  class Class1 {
   public:
    VERSIONED_ENUM(Parameters, i, j);
  };

  class Class2 {
   public:
    VERSIONED_ENUM(Parameters, i, j, k);
  };

}


TEST(VersionedEnumTest, ChecksumTest) {
  EXPECT_NE(Class1::ParametersVersion(),
            Class2::ParametersVersion());
}

