#include <gtest/gtest.h>

#include <cmath>

namespace release_mode {

#ifndef NDEBUG
#define NDEBUG
#endif
#include "invalidate.h"

TEST(InvalidateTest, ReleaseTest1) {
    float var = 7;
    InvalidateScalarInDebug(&var);
    EXPECT_EQ(7, var);
}

TEST(InvalidateTest, ReleaseTest2) {
    float var = 7;
    InvalidateScalar(&var);
    EXPECT_TRUE(std::isnan(var));
}


}  // namespace release_mode

namespace debug_mode {
#ifdef NDEBUG
#undef NDEBUG
#endif

#undef COMMON_INVALIDATE_H
#include "invalidate.h"

TEST(InvalidateTest, ReleaseTest1) {
    float var = 7;
    InvalidateScalar(&var);
    EXPECT_TRUE(std::isnan(var));
}

TEST(InvalidateTest, ReleaseTest2) {
    float var = 7;
    InvalidateScalarInDebug(&var);
    EXPECT_TRUE(std::isnan(var));
}

}  // namespace debug_mode
