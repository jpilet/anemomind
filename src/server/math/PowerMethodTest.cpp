/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/math/PowerMethod.h>
#include <gtest/gtest.h>
#include <server/common/string.h>

using namespace sail;
using namespace sail::PowerMethod;

TEST(PowerMethodTest, SmallTest) {

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

TEST(PowerMethodTest, LargeTest) {
  Eigen::MatrixXd KtK(9, 9);
  KtK(0, 0) = 13.286712999783560;
  KtK(0, 1) = -2.082147412252239;
  KtK(0, 2) = -0.769560317095434;
  KtK(0, 3) = -0.616008548998472;
  KtK(0, 4) = -2.158391763461411;
  KtK(0, 5) = -1.923571906432537;
  KtK(0, 6) = -4.956624713810227;
  KtK(0, 7) = -2.071222395859969;
  KtK(0, 8) = 0.821767891075802;
  KtK(1, 0) = -2.082147412252239;
  KtK(1, 1) = 12.688440388800572;
  KtK(1, 2) = 4.921318459067312;
  KtK(1, 3) = 1.312229484845397;
  KtK(1, 4) = 0.918584209258105;
  KtK(1, 5) = -2.815999524573849;
  KtK(1, 6) = 7.662573616300280;
  KtK(1, 7) = 5.904551189268731;
  KtK(1, 8) = 1.742414832024328;
  KtK(2, 0) = -0.769560317095434;
  KtK(2, 1) = 4.921318459067312;
  KtK(2, 2) = 11.222769743639819;
  KtK(2, 3) = 0.462841776087780;
  KtK(2, 4) = -0.078615095293722;
  KtK(2, 5) = -6.050687829269147;
  KtK(2, 6) = 1.937370723399825;
  KtK(2, 7) = -6.158311853217532;
  KtK(2, 8) = -0.683123948032082;
  KtK(3, 0) = -0.616008548998472;
  KtK(3, 1) = 1.312229484845397;
  KtK(3, 2) = 0.462841776087780;
  KtK(3, 3) = 4.942256926326965;
  KtK(3, 4) = 0.881573331991671;
  KtK(3, 5) = -0.231989233712308;
  KtK(3, 6) = 0.001364925701209;
  KtK(3, 7) = -0.218268766196987;
  KtK(3, 8) = -1.804364012724338;
  KtK(4, 0) = -2.158391763461411;
  KtK(4, 1) = 0.918584209258105;
  KtK(4, 2) = -0.078615095293722;
  KtK(4, 3) = 0.881573331991671;
  KtK(4, 4) = 3.223192866712450;
  KtK(4, 5) = -0.290524604299499;
  KtK(4, 6) = 0.849853909822685;
  KtK(4, 7) = -0.113654483002618;
  KtK(4, 8) = -1.756257937576114;
  KtK(5, 0) = -1.923571906432537;
  KtK(5, 1) = -2.815999524573849;
  KtK(5, 2) = -6.050687829269147;
  KtK(5, 3) = -0.231989233712308;
  KtK(5, 4) = -0.290524604299499;
  KtK(5, 5) = 9.574723045933620;
  KtK(5, 6) = 1.102790064595685;
  KtK(5, 7) = 7.090023062326183;
  KtK(5, 8) = 0.516320011421111;
  KtK(6, 0) = -4.956624713810227;
  KtK(6, 1) = 7.662573616300280;
  KtK(6, 2) = 1.937370723399825;
  KtK(6, 3) = 0.001364925701209;
  KtK(6, 4) = 0.849853909822685;
  KtK(6, 5) = 1.102790064595685;
  KtK(6, 6) = 16.395555105793395;
  KtK(6, 7) = 10.695169936429487;
  KtK(6, 8) = 5.219382575355834;
  KtK(7, 0) = -2.071222395859969;
  KtK(7, 1) = 5.904551189268731;
  KtK(7, 2) = -6.158311853217532;
  KtK(7, 3) = -0.218268766196987;
  KtK(7, 4) = -0.113654483002618;
  KtK(7, 5) = 7.090023062326183;
  KtK(7, 6) = 10.695169936429487;
  KtK(7, 7) = 17.001412804196239;
  KtK(7, 8) = 6.282188628110267;
  KtK(8, 0) = 0.821767891075802;
  KtK(8, 1) = 1.742414832024328;
  KtK(8, 2) = -0.683123948032082;
  KtK(8, 3) = -1.804364012724338;
  KtK(8, 4) = -1.756257937576114;
  KtK(8, 5) = 0.516320011421111;
  KtK(8, 6) = 5.219382575355834;
  KtK(8, 7) = 6.282188628110267;
  KtK(8, 8) = 10.178615901155947;

  Eigen::VectorXd gtMinVec(9);
  gtMinVec(0) = -0.062469773882297;
  gtMinVec(1) = -0.408836340317720;
  gtMinVec(2) = 0.405461506293981;
  gtMinVec(3) = 0.005912817791833;
  gtMinVec(4) = 0.063070945111740;
  gtMinVec(5) = -0.353880782019045;
  gtMinVec(6) = -0.248526568965855;
  gtMinVec(7) = 0.668687113713795;
  gtMinVec(8) = -0.162477432764556;

  double gtMinVal = 0.584475743524517;

  Settings s;
  s.maxIters = 1000;
  s.tol = Angle<double>::radians(0.0000001);

  auto results = PowerMethod::computeMin([&](const Eigen::VectorXd &x) {
    return KtK*x;
  }, Eigen::VectorXd::Ones(9), s);

  double minVal = results.X.norm();
  Eigen::VectorXd X = (1.0/minVal)*results.X;
  EXPECT_NEAR(sinAngle(results.X, gtMinVec), 0, 1.0e-5);
  EXPECT_NEAR(minVal, gtMinVal, 1.0e-6);
}






