#include <server/common/Uniform.h>
#include <server/plot/extra.h>
#include <server/math/nonlinear/GridFitter.h>
#include <server/math/Grid.h>
#include <server/math/ADFunction.h>
#include <server/common/logging.h>
#include "NoisyStep.h"
#include <server/common/string.h>
#include <server/common/ArrayIO.h>
#include <server/common/DataSplits.h>

namespace sail {




void sfitexFixedReg() {
  BBox1d bbox(Spand(-1.0, 1.0));
  double spacing[1] = {0.03};
  Grid1d grid(bbox, spacing);

  arma::sp_mat A = grid.makeFirstOrderReg(0);

  int sampleCount = 30;
  Arrayd X, Ygt, Ynoisy;
  makeNoisySignalData(sampleCount, X, Ygt, Ynoisy);

  arma::sp_mat P = grid.makeP(MDArray2d(X));

  {
    // Validate P
    MDArray2d V = grid.getGridVertexCoords();
    arma::mat Vmat(V.getData(), V.rows(), V.cols(), false, true);
    arma::mat Xmat(X.getData(), X.size(), 1, false, true);
    assert(arma::norm(P*Vmat - Xmat, 2) <= 1.0e-6);
  }

  Array<Arrayb> splits = makeRandomSplits(3, X.size());

  NoisyStep data(X, Ynoisy);

  GridFitter gridFitter;


  std::shared_ptr<GridFit> gf(new GridFit(P, &data, Array<arma::sp_mat>::args(A), splits, Arrayd::args(10.0)));
  gridFitter.add(gf);

  std::shared_ptr<MatExpr> Pinv = gf->makeDataToParamMat();


  arma::mat params(1, 1);
  params[0] = 30.0;
  gridFitter.solveFixedReg(&params);
  LOG(INFO) << EXPR_AND_VAL_AS_STRING(params);
  LOG(INFO) << "Done";

  Arrayd Yfitted(sampleCount);
  data.eval(params.memptr(), Yfitted.getData());
  arma::mat vertices = Pinv->mulWithDense(arma::mat(Yfitted.getData(), Yfitted.size(), 1, false, true));

  GnuplotExtra plot;
  plot.set_style("lines");
  plot.plot_xy(X, Ynoisy, "Noisy input");
  plot.plot_xy(X, Yfitted, "Non-linear transformation of noisy signal");
  plot.set_style("linespoints");
  plot.plot_xy(grid.getGridVertexCoords().getStorage(), Arrayd(vertices.n_elem, vertices.memptr()), "Fitted grid");
  plot.show();
}



void sfitexAutoRegFirstOrder() {
  BBox1d bbox(Spand(-1.0, 1.0));
  double spacing[1] = {0.03};
  Grid1d grid(bbox, spacing);

  arma::sp_mat A = grid.makeFirstOrderReg(0);

  int sampleCount = 30;
  Arrayd X, Ygt, Ynoisy;
  makeNoisySignalData(sampleCount, X, Ygt, Ynoisy);
  arma::sp_mat P = grid.makeP(MDArray2d(X));

  {
    // Validate P
    MDArray2d V = grid.getGridVertexCoords();
    arma::mat Vmat(V.getData(), V.rows(), V.cols(), false, true);
    arma::mat Xmat(X.getData(), X.size(), 1, false, true);
    assert(arma::norm(P*Vmat - Xmat, 2) <= 1.0e-6);
  }

  Array<Arrayb> splits = makeRandomSplits(9, X.size());

  NoisyStep data(X, Ynoisy);

  GridFitter gridFitter;


  double initReg = 1.0; // works
  //double initReg = 1000.0;

  std::shared_ptr<GridFit> gf(new GridFit(P, &data, Array<arma::sp_mat>::args(A), splits, Arrayd::args(initReg)));
  gridFitter.add(gf);

  arma::mat params(1, 1);
  params[0] = 3000.0;
  gridFitter.solve(&params);


  std::shared_ptr<MatExpr> Pinv = gf->makeDataToParamMat();


  Arrayd Yfitted(sampleCount);
  data.eval(params.memptr(), Yfitted.getData());
  arma::mat D(Yfitted.getData(), Yfitted.size(), 1, false, true);
  arma::mat vertices = Pinv->mulWithDense(D);
  LOG(INFO) << EXPR_AND_VAL_AS_STRING(gf->getRegWeight(0));
  LOG(INFO) << "Done";

  GnuplotExtra plot;
  plot.set_style("lines");
  plot.plot_xy(X, Ynoisy, "Noisy input");
  plot.plot_xy(X, Yfitted, "Non-linear transformation of noisy signal");
  plot.set_style("linespoints");
  plot.plot_xy(grid.getGridVertexCoords().getStorage(), Arrayd(vertices.n_elem, vertices.memptr()), "Fitted grid");
  plot.show();

}


void sfitexAutoReg1st2ndOrder() {
  BBox1d bbox(Spand(-1.0, 1.0));
  double spacing[1] = {0.03};
  Grid1d grid(bbox, spacing);

  arma::sp_mat A1 = grid.makeFirstOrderReg(0);
  arma::sp_mat A2 = grid.makeSecondOrderReg(0);

  int sampleCount = 30;
  Arrayd X, Ygt, Ynoisy;
  makeNoisySignalData(sampleCount, X, Ygt, Ynoisy);

  arma::sp_mat P = grid.makeP(MDArray2d(X));

  Array<Arrayb> splits = makeRandomSplits(9, X.size());

  //double initReg = 0.01; // works
  //double initReg = 1; // works
  double initReg = 0.1;


  NoisyStep data(X, Ynoisy);
  GridFitter gridFitter;
  std::shared_ptr<GridFit> gf(new GridFit(P, &data, Array<arma::sp_mat>::args(A1, A2), splits, Arrayd::args(initReg, initReg)));
  gridFitter.add(gf);

  arma::mat params(1, 1);
  params[0] = 3000.0;
  gridFitter.solve(&params);

  std::shared_ptr<MatExpr> Pinv = gf->makeDataToParamMat();

  Arrayd Yfitted(sampleCount);
  data.eval(params.memptr(), Yfitted.getData());
  arma::mat D(Yfitted.getData(), Yfitted.size(), 1, false, true);
  arma::mat vertices = Pinv->mulWithDense(D);

  GnuplotExtra plot;
  plot.set_style("lines");
  plot.plot_xy(X, Ynoisy, "Ground-truth noisy data vector (with correct calibration)");
  plot.plot_xy(X, Yfitted, "Data vector with estimated calibration");
  plot.set_style("linespoints");
  plot.plot_xy(grid.getGridVertexCoords().getStorage(),
               Arrayd(vertices.n_elem, vertices.memptr()), "Fitted model grid");
  plot.show();
}

}

