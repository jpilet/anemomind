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


namespace {
  class GridFitterTestData {
   public:
    GridFitterTestData();

    int sampleCount;
    Arrayd X, Ygt, Ynoisy;

    Array<Arrayb> splits;
  };
}

GridFitterTestData::GridFitterTestData() {
  const int sampleCount_ = 30;
  double Xdata[sampleCount_] = {-0.967399, -0.782382, -0.740419, -0.725537, -0.716795, -0.686642, -0.604897, -0.563486, -0.514226, -0.444451, -0.329554, -0.270431, -0.211234, -0.198111, -0.0452059, 0.0268018, 0.10794, 0.213938, 0.257742, 0.271423, 0.434594, 0.536459, 0.566198, 0.59688, 0.608354, 0.680375, 0.823295, 0.83239, 0.904459, 0.997849};
  double Ygtdata[sampleCount_] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  double Ynoisydata[sampleCount_] = {-0.994827, -0.864355, -0.954944, -1.08159, -0.944979, -0.990285, -1.00257, -0.81089, -1.08299, -0.891457, -0.989302, -0.892034, -1.03991, -0.843388, -1.08667, 0.940983, 1.12309, 1.16761, 0.827902, 1.17973, 1.0104, 0.834422, 0.876886, 1.06529, 1.15609, 0.939557, 0.825669, 0.808009, 0.983081, 0.825238};
  X = Arrayd(sampleCount_, Xdata).dup();
  Ygt = Arrayd(sampleCount_, Ygtdata).dup();
  Ynoisy = Arrayd(sampleCount_, Ynoisydata).dup();
  sampleCount = sampleCount_;

  bool splitdata0[sampleCount_] = {1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0};
  bool splitdata1[sampleCount_] = {1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0};
  bool splitdata2[sampleCount_] = {0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1};
  bool splitdata3[sampleCount_] = {1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1};
  bool splitdata4[sampleCount_] = {0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1};
  bool splitdata5[sampleCount_] = {1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1};
  bool splitdata6[sampleCount_] = {0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0};
  bool splitdata7[sampleCount_] = {1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0};
  bool splitdata8[sampleCount_] = {1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0};

  const int splitCount = 9;
  Arrayb splitsdata[splitCount] = {Arrayb(sampleCount_, splitdata0).dup(),
      Arrayb(sampleCount_, splitdata1).dup(),
      Arrayb(sampleCount_, splitdata2).dup(),
      Arrayb(sampleCount_, splitdata3).dup(),
      Arrayb(sampleCount_, splitdata4).dup(),
      Arrayb(sampleCount_, splitdata5).dup(),
      Arrayb(sampleCount_, splitdata6).dup(),
      Arrayb(sampleCount_, splitdata7).dup(),
      Arrayb(sampleCount_, splitdata8).dup()};
   splits = Array<Arrayb>(splitCount, splitsdata).dup();
}


TEST(GridFitterTest, TestFixedReg) {
  BBox1d bbox(Span(-1.0, 1.0));
  double spacing[1] = {0.03};
  Grid1d grid(bbox, spacing);

  arma::sp_mat A = grid.makeFirstOrderReg(0);

  GridFitterTestData td;


  arma::sp_mat P = grid.makeP(MDArray2d(td.X));

  {
    // Validate P
    MDArray2d V = grid.getGridVertexCoords();
    arma::mat Vmat(V.getData(), V.rows(), V.cols(), false, true);
    arma::mat Xmat(td.X.getData(), td.X.size(), 1, false, true);
    assert(arma::norm(P*Vmat - Xmat, 2) <= 1.0e-6);
  }

  NoisyStep data(td.X, td.Ynoisy);

  GridFitter gridFitter;


  std::shared_ptr<GridFit> gf(new GridFit(P, &data, Array<arma::sp_mat>::args(A), td.splits, Arrayd::args(100.0)));
  gridFitter.add(gf);

  std::shared_ptr<MatExpr> Pinv = gf->makeDataToParamMat();


  arma::mat params(1, 1);
  params[0] = 30.0;
  gridFitter.solveFixedReg(&params);

  Arrayd Yfitted(td.sampleCount);
  data.eval(params.memptr(), Yfitted.getData());
  int counter = 0;
  for (int i = 0; i < td.sampleCount; i++) {
    counter += ((Yfitted[i] > 0) == (td.Ygt[i] > 0)? 1 : 0);
  }

  double tol = (0.25*td.sampleCount);
  EXPECT_NEAR(counter, td.sampleCount, tol);
}


TEST(GridFitterTest, TestAutoTuneFirstOrder) { // Based on the example: SignalFitExampleAutoRegFirstOrder.cpp
  ScopedLog::setDepthLimit(0);
  BBox1d bbox(Span(-1.0, 1.0));
  double spacing[1] = {0.03};
  Grid1d grid(bbox, spacing);

  arma::sp_mat A = grid.makeFirstOrderReg(0);

  GridFitterTestData td;


  arma::sp_mat P = grid.makeP(MDArray2d(td.X));

  {
    // Validate P
    MDArray2d V = grid.getGridVertexCoords();
    arma::mat Vmat(V.getData(), V.rows(), V.cols(), false, true);
    arma::mat Xmat(td.X.getData(), td.X.size(), 1, false, true);
    EXPECT_NEAR(arma::norm(P*Vmat - Xmat, 2), 0.0, 1.0e-6);
  }


  NoisyStep data(td.X, td.Ynoisy);

  GridFitter gridFitter;


  double initReg = 1.0; // works

  std::shared_ptr<GridFit> gf(new GridFit(P, &data, Array<arma::sp_mat>::args(A), td.splits, Arrayd::args(initReg)));
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


  Arrayd Yfitted(td.sampleCount);
  data.eval(params.memptr(), Yfitted.getData());
  arma::mat D(Yfitted.getData(), Yfitted.size(), 1, false, true);
  for (int i = 0; i < td.sampleCount; i++) {
    EXPECT_NEAR(Yfitted[i], td.Ygt[i], 0.25); // Be quite tolerant here
  }
}
