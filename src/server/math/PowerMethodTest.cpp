/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/math/PowerMethod.h>
#include <gtest/gtest.h>

using namespace sail;
using namespace sail::PowerMethod;

TEST(LinearOptCalib, PowerMethodTest) {

  Eigen::MatrixXd KtK(2, 2);
  KtK(0, 0) =   0.389749364074819;
  KtK(1, 0) = 0.272310680013061;
  KtK(0, 1) = 0.272310680013061;
  KtK(1, 1) =  6.64420869549255;

  Eigen::VectorXd gtMaxVec(2), gtMinVec(2);
  gtMinVec(0) = -0.999057104893893;
  gtMaxVec(0) = 0.0434154484145177;
  gtMinVec(1) = 0.0434154484145177;
  gtMaxVec(1) = 0.999057104893893;

  double gtMinVal = 0.377915715905043;
  double gtMaxVal = 6.65604234366233;

  Eigen::VectorXd Xinit(2);
  Xinit(0) = 0.5;
  Xinit(1) = 0.5;

  std::function<Eigen::VectorXd(Eigen::VectorXd)> matMul = [&](Eigen::VectorXd x) {
      return KtK*x;
    };

  PowerMethod::Settings s;
  auto maxVecData = PowerMethod::computeMax(matMul, Xinit, s);

  double maxEigVal = maxVecData.X.norm();
  EXPECT_NEAR(maxEigVal, gtMaxVal, 1.0e-6);

  EXPECT_LE(sinAngle(maxVecData.X, gtMaxVec), 1.0e-5);

  Eigen::MatrixXd B = KtK - maxEigVal*Eigen::MatrixXd::Identity(2, 2);


  auto minVecData = PowerMethod::computeMax([&](Eigen::VectorXd x) {
    auto tmp = matMul(x);
    /*Eigen::VectorXd results = tmp - maxEigVal*x;
    return results;*/
    return Eigen::VectorXd(tmp - maxEigVal*x);
  }, Xinit, s);

  Eigen::VectorXd minVec = (1.0/minVecData.X.norm())*KtK*minVecData.X;
  EXPECT_NEAR(minVec.norm(), gtMinVal, 1.0e-6);

  auto minData2 = PowerMethod::computeMin(matMul, Xinit, s);
  EXPECT_NEAR(minData2.X.norm(), gtMinVal, 1.0e-6);
  EXPECT_LE(sinAngle(minData2.X, gtMinVec), 1.0e-5);
}




