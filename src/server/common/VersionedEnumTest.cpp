/*
 *  Created on: 2014-07-01
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/VersionedEnum.h>
#include <gtest/gtest.h>

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

using namespace sail;

TEST(VersionedEnumTest, ChecksumTest) {
  EXPECT_NE(calcChecksum(Class1::ParametersTextRepr),
            calcChecksum(Class1::ParametersTextRepr));
}

