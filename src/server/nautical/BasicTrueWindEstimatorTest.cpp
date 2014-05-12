#include "BasicTrueWindEstimator.h"

#include <gtest/gtest.h>
#include <server/common/Env.h>
#include <server/common/logging.h>
#include <server/nautical/NavNmea.h>
#include <string>

using std::string;
using namespace sail;

TEST(BasicTrueWindEstimatorTest, SmokeTest) {
    Array<Nav> navs = loadNavsFromNmea(
       string(Env::SOURCE_DIR) + string("/datasets/tinylog.txt"),
       Nav::Id("B0A10000")).navs(); 

    CHECK_LT(0, navs.size());

    double parameters[BasicTrueWindEstimator::NUM_PARAMS];
    BasicTrueWindEstimator::initializeParameters(parameters);

    auto trueWind = BasicTrueWindEstimator::computeTrueWind(parameters, navs);

    LOG(INFO) << trueWind[0].knots() << ", " << trueWind[1].knots();
}

