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
#include <server/math/mathutils.h>
#include <server/math/nonlinear/GridFitterTestData.h>
using namespace sail;








TEST(GridFitterTest, TestFixedReg) {
  ScopedLog::setDepthLimit(0);
  BBox1d bbox(Spand(-1.0, 1.0));
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


TEST(GridFitterTest, TestAutoTuneFirstOrder) {
  ScopedLog::setDepthLimit(0);
  BBox1d bbox(Spand(-1.0, 1.0));
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


  Arrayd Yfitted(td.sampleCount);
  data.eval(params.memptr(), Yfitted.getData());
  for (int i = 0; i < td.sampleCount; i++) {
    EXPECT_NEAR(Yfitted[i], td.Ygt[i], 0.25); // Be quite tolerant here
  }
}




TEST(GridFitterTest, TestAutoTune1stAnd2ndOrder) {
  ScopedLog::setDepthLimit(0);
  BBox1d bbox(Spand(-1.0, 1.0));
  double spacing[1] = {0.03};
  Grid1d grid(bbox, spacing);

  arma::sp_mat A1 = grid.makeFirstOrderReg(0);
  arma::sp_mat A2 = grid.makeSecondOrderReg(0);

  GridFitterTestData td;

  arma::sp_mat P = grid.makeP(MDArray2d(td.X));

  //double initReg = 0.01; // works
  //double initReg = 1; // works
  double initReg = 0.1;


  NoisyStep data(td.X, td.Ynoisy);
  GridFitter gridFitter;
  std::shared_ptr<GridFit> gf(new GridFit(P, &data, Array<arma::sp_mat>::args(A1, A2), td.splits, Arrayd::args(initReg, initReg)));
  gridFitter.add(gf);

  arma::mat params(1, 1);
  params[0] = 3000.0;
  gridFitter.solve(&params);


  Arrayd Yfitted(td.sampleCount);
  data.eval(params.memptr(), Yfitted.getData());

  int counter = 0;
  for (int i = 0; i < td.sampleCount; i++) {
    counter += ((Yfitted[i] > 0) == (td.Ygt[i] > 0)? 1 : 0);
  }

  double tol = (0.25*td.sampleCount);
  EXPECT_NEAR(counter, td.sampleCount, tol);
}
