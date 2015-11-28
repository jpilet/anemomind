/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/calibration/RegCov.h>
#include <server/nautical/calibration/LinearCalibration.h>
#include <server/nautical/Nav.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/NavNmeaScan.h>
#include <server/common/logging.h>
#include <server/math/EigenUtils.h>
#include <server/common/ArrayIO.h>

using namespace sail;
using namespace LinearCalibration;
using namespace EigenUtils;


namespace {
  auto rng = makeRngForTests();

  bool eq(const FlowFiber &a, const FlowFiber &b, double tol = 1.0e-6) {
    return EigenUtils::eq(a.Q, b.Q, tol) && EigenUtils::eq(a.B, b.B, tol);
  }
}

TEST(RegCovTest, AccumulateTrajectory) {
  Arrayd X{9, 2, 3, 4};
  Arrayd Yexpected{0, 0, 9, 2, 12, 6};
  EXPECT_EQ(Yexpected, accumulateTrajectory(X));
}

TEST(RegCovTest, RegTest) {
  Arrayd trajectory{0, 0, 1, 1, 0, 0, 3, 2, 0, 0, 7, 9};
  double expected = sqrt(sqr(1 - 2*3 + 7) + sqr(1 - 2*2 + 9));
  auto reg = calcReg(trajectory, 1, 2, 0.0);
  EXPECT_NEAR(reg, expected, 1.0e-9);
  int step = 2;
  auto regs = computeRegs(trajectory, step);
  EXPECT_EQ(regs.size(), 3);
  auto difs = computeDifs(regs);
  EXPECT_EQ(Arrayd(difs), Arrayd(computeRegDifs(trajectory, step)));
  EXPECT_EQ(difs.size(), computeDifCount(getDataCount(trajectory.size()), step));

  EXPECT_NEAR(computeCovariance(Arrayd{0.0, 1.0, 0.0, 1.0},
                                Arrayd{0.0, 0.0, 1.0, 1.0},
                                Arrayi{0, 1, 2, 3}),
              0.0,
              1.0e-6);
  EXPECT_NEAR(computeCovariance(Arrayd{0.0, 1.0},
                                Arrayd{0.0, 1.0},
                                Arrayi{0, 1}),
              0.5,
              1.0e-6);
}

Array<Nav> getTestDataset() {
  auto p = PathBuilder::makeDirectory(Env::SOURCE_DIR)
    .pushDirectory("datasets")
    .pushDirectory("Irene")
    .pushDirectory("2013").get();
  auto allNavs = scanNmeaFolder(p, Nav::debuggingBoatId());
  auto from = TimeStamp::UTC(2013, 8, 2, 10, 8, 0);
  auto to = TimeStamp::UTC(2013, 8, 2, 13, 41, 44);
  auto navs = allNavs.slice([=](const Nav &x) {
    auto t = x.time();
    return from < t && t < to;
  });
  LOG(INFO) << "Selected " << navs.size() << " navs";
  return navs;
}

TEST(RegCovTest, TestIntegration) {
  int step = 100;
  int samplesPerSplit = 100;

  auto ds = getTestDataset();
  int difCount = computeDifCount(ds.size(), step);
  int splitCount = difCount/samplesPerSplit;

  Arrayd Xinit = makeXinit();
  EXPECT_EQ(Xinit.size(), 4);
  Array<Arrayi> splits = makeRandomSplit(difCount, splitCount, &rng);
}

