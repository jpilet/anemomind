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
#include <server/nautical/calib/Correction.h>

namespace {

  using namespace sail;
  using namespace CornerCalibTestData;

#pragma pack(push, 1)
  template <typename T>
  struct TestParams {
    THIS_PARAM_DIM(TestParams)

    T speedBias = T(1.0);
    T speedOffset = T(0.0);
    T angleOffset = T(0.0);

  };
#pragma pack(pop)


  class TestFlowFun {
  public:
    static const int FlowCount = 1;
    typedef TestParams<double> InitialParamType;

    HorizontalMotion<double> getRefMotion(const TestSample &sample) {
      return sample.refMotion();
    }

    template <typename T>
    std::array<HorizontalMotion<T>, 1> apply(
        const T *params0, const TestSample &sample) const {
      const auto &params = *(reinterpret_cast<const TestParams<T> *>(params0));
      return std::array<HorizontalMotion<T>, 1>{computeCurrentFromBoatMotionOverWaterAndGround<HorizontalMotion<T> >(
          correctOrCorruptVector(
              sample.corruptedMotionOverWaterVec(),
              params.speedBias, params.speedOffset, params.angleOffset),
              sample.boatMotionVec.cast<T>())};
    }
  };

  Velocity<double> computeAverageCurrentError(
      TestFlowFun f,
      const Array<TestSample> &samples, TestParams<double> params) {
    auto sum = Velocity<double>::knots(0.0);
    for (auto sample: samples) {
      auto estCurrent = f.apply(reinterpret_cast<double *>(&params), sample)[0];
      sum += HorizontalMotion<double>(estCurrent - getTrueConstantCurrent()).norm();
    }
    return (1.0/samples.size())*sum;
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

  TestFlowFun::InitialParamType defaultParams;
  auto params = optimizeCornernessForGroups<TestSample, TestFlowFun>(f,
      Array<Array<TestSample> >{samples}, settings);

  auto initErr = computeAverageCurrentError(f, samples, defaultParams);
  EXPECT_LE(2.6, initErr.knots());
  auto optErr = computeAverageCurrentError(f, samples, params);
  EXPECT_LE(optErr.knots(), 0.07);
  std::cout << "initErr: " << initErr.knots() << std::endl;
  std::cout << "optErr: " << optErr.knots() << std::endl;
}



