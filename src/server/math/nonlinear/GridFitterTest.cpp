/*
 *  Created on: 2014-02-26
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/BBox.h>
#include <server/math/Grid.h>
#include <server/math/nonlinear/NoisyStep.h>
#include <server/math/nonlinear/GridFitter.h>
#include <server/common/ArrayIO.h>
#include <server/common/ScopedLog.h>

using namespace sail;

TEST(GridFitterTest, TestAutoTuneFirstOrder) { // Based on the example: SignalFitExampleAutoRegFirstOrder.cpp
  ScopedLog::setDepthLimit(0);
  BBox1d bbox(Span(-1.0, 1.0));
  double spacing[1] = {0.03};
  Grid1d grid(bbox, spacing);

  arma::sp_mat A = grid.makeFirstOrderReg(0);

  const int sampleCount = 30;

  double Xdata[sampleCount] = {-0.967399, -0.782382, -0.740419, -0.725537, -0.716795, -0.686642, -0.604897, -0.563486, -0.514226, -0.444451, -0.329554, -0.270431, -0.211234, -0.198111, -0.0452059, 0.0268018, 0.10794, 0.213938, 0.257742, 0.271423, 0.434594, 0.536459, 0.566198, 0.59688, 0.608354, 0.680375, 0.823295, 0.83239, 0.904459, 0.997849};
  double Ygtdata[sampleCount] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  double Ynoisydata[sampleCount] = {-0.994827, -0.864355, -0.954944, -1.08159, -0.944979, -0.990285, -1.00257, -0.81089, -1.08299, -0.891457, -0.989302, -0.892034, -1.03991, -0.843388, -1.08667, 0.940983, 1.12309, 1.16761, 0.827902, 1.17973, 1.0104, 0.834422, 0.876886, 1.06529, 1.15609, 0.939557, 0.825669, 0.808009, 0.983081, 0.825238};

  Arrayd X(sampleCount, Xdata);
  Arrayd Ygt(sampleCount, Ygtdata);
  Arrayd Ynoisy(sampleCount, Ynoisydata);


  arma::sp_mat P = grid.makeP(MDArray2d(X));

  {
    // Validate P
    MDArray2d V = grid.getGridVertexCoords();
    arma::mat Vmat(V.getData(), V.rows(), V.cols(), false, true);
    arma::mat Xmat(X.getData(), X.size(), 1, false, true);
    EXPECT_NEAR(arma::norm(P*Vmat - Xmat, 2), 0.0, 1.0e-6);
  }

  bool splitdata0[sampleCount] = {1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0};
  bool splitdata1[sampleCount] = {1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0};
  bool splitdata2[sampleCount] = {0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1};
  bool splitdata3[sampleCount] = {1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1};
  bool splitdata4[sampleCount] = {0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1};
  bool splitdata5[sampleCount] = {1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1};
  bool splitdata6[sampleCount] = {0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0};
  bool splitdata7[sampleCount] = {1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0};
  bool splitdata8[sampleCount] = {1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0};

  const int splitCount = 9;
  Arrayb splitsdata[splitCount] = {Arrayb(sampleCount, splitdata0),
      Arrayb(sampleCount, splitdata1),
      Arrayb(sampleCount, splitdata2),
      Arrayb(sampleCount, splitdata3),
      Arrayb(sampleCount, splitdata4),
      Arrayb(sampleCount, splitdata5),
      Arrayb(sampleCount, splitdata6),
      Arrayb(sampleCount, splitdata7),
      Arrayb(sampleCount, splitdata8)};

  Array<Arrayb> splits(splitCount, splitsdata);

  NoisyStep data(X, Ynoisy);

  GridFitter gridFitter;


  double initReg = 1.0; // works

  std::shared_ptr<GridFit> gf(new GridFit(P, &data, Array<arma::sp_mat>::args(A), splits, Arrayd::args(initReg)));
  gridFitter.add(gf);

  arma::mat params(1, 1);
  params[0] = 3000.0;
  gridFitter.solve(&params);

  // Since we are dealing with randomized data
  // and small tweaks to the optmizer could make us converge to
  // a slightly different equilibrium point, don't require
  // a too precise answer here.
  EXPECT_NEAR(params[0], 0.0, 0.1);
  EXPECT_NEAR(gf->getRegWeight(0), 1.6, 0.5);

  std::shared_ptr<MatExpr> Pinv = gf->makeDataToParamMat();


  Arrayd Yfitted(sampleCount);
  data.eval(params.memptr(), Yfitted.getData());
  arma::mat D(Yfitted.getData(), Yfitted.size(), 1, false, true);
  arma::mat vertices = Pinv->mulWithDense(D);
  for (int i = 0; i < sampleCount; i++) {
    EXPECT_NEAR(Yfitted[i], Ygt[i], 0.25); // Be quite tolerant here
  }
}
