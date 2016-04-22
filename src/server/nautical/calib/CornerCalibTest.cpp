/*
 * CornerCalibTest.cpp
 *
 *  Created on: Apr 15, 2016
 *      Author: jonas
 */

#include <server/nautical/calib/CornerCalib.h>
#include <server/nautical/calib/CornerCalibTestData.h>
#include <gtest/gtest.h>
#include <server/nautical/common.h>

namespace {

  using namespace sail;
  using namespace CornerCalibTestData;


  class TestFlowFun {
  public:
    Arrayd initialParams() const {
      return Arrayd{1.0, 0.0, 0.0};
    }

    template <typename T>
    Eigen::Matrix<T, 2, 1> evalFlow(
        const TestSample &sample, const T *params) const {
      return computeCurrentFromBoatMotion<Eigen::Matrix<T, 2, 1> >(
          correctOrCorruptVector(
              sample.corruptedMotionOverWaterVec(),
              params[0], params[1], params[2]),
              sample.boatMotionVec.cast<T>());
    }
  };

  double computeAverageCurrentError(
      TestFlowFun f,
      const Array<TestSample> &samples, Arrayd params) {
    double sum = 0.0;
    for (auto sample: samples) {
      auto estCurrent = f.evalFlow(sample, params.ptr());
      sum += (estCurrent - getTrueConstantCurrent()).norm();
    }
    return sum/samples.size();
  }
}

TEST(CornerCalib, BasicTest) {
  auto corruptParams = sail::CornerCalibTestData::getDefaultCorruptParams();

  auto samples = makeTestSamples(corruptParams);

  using namespace CornerCalib;
  using namespace sail::CornerCalib;

  Settings settings;
  settings.ceresOptions.minimizer_progress_to_stdout = false;
  settings.ceresOptions.logging_type = ceres::SILENT;
  settings.windowSize = 16;

  TestFlowFun f;
  auto defaultParams = f.initialParams();
  auto params = optimizeCornerness<TestSample, TestFlowFun>(f, samples, settings);

  auto initErr = computeAverageCurrentError(f, samples, defaultParams);
  EXPECT_LE(2.6, initErr);
  auto optErr = computeAverageCurrentError(f, samples, params);
  EXPECT_LE(optErr, 0.07);
  std::cout << "initErr: " << initErr << std::endl;
  std::cout << "optErr: " << optErr << std::endl;
}



