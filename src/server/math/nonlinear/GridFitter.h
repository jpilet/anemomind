/*
 * GridFitter.h
 *
 *  Created on: 2014-02-20
 *      Author: jonas
 */

#ifndef GRIDFITTER_H_
#define GRIDFITTER_H_

#include <armadillo>
#include <list>
#include <memory>
#include <server/common/Array.h>
#include <vector>
#include <server/math/nonlinear/LevmarSettings.h>
#include <server/math/MatExpr.h>

namespace sail {

// This class holds one nonlinearly parameterized datavector to which we want to fit a grid and tune
// regularization parameters.

class AutoDiffFunction;
class GridFit {
 public:
  GridFit();
  GridFit(arma::sp_mat P, AutoDiffFunction *data, Array<arma::sp_mat> regMatrices,
          Array<Arrayb> splits,
          Arrayd regWeights,
          Array<std::string> regWeightLabels = Array<std::string>(),
          double weight = 1);


  AutoDiffFunction &getData() {
    return *_data;
  }

  int getNLParamCount() const;

  // Selects and slices a subset of the rows of _P
  arma::sp_mat makePsel(Arrayb sel);


  // Returns a matrix that, when multiplied by a data vector,
  // gives the least squares fit of the grid vertices to that data vector.
  // Optionally also takes a boolean array that select which elements of
  // the data vector to be used for the fit.
  std::shared_ptr<MatExpr> makeDataToParamMat(Arrayb sel = Arrayb());

  // Takes the above matrix and multiplies it with the various cost matrices inside
  // If D is the datavector and this function returns a matrix A,
  // then we want to minimize |A*D| w.r.t. D
  std::shared_ptr<MatExpr> makeDataToResidualsMat(Arrayb sel = Arrayb());

  // For the current weights, return a matrix M such that
  // |M*D|^2 is the cross validation cost, D being the data vector.
  std::shared_ptr<MatExpr> makeCrossValidationFitnessMat();



  // Evaluates how well the grid fits to a datavector D
  // given the current regularization weights.
  // The result of this function should be the same as
  // SQNORM(makeDataToResidualMat()*D) but is computed differently
  double evalObjfForDataVector(arma::mat D);

  // Evaluates the cross-validation score for this datavector D
  // and the weights.
  double evalCrossValidationFitness(arma::mat D);

  void setExpRegWeight(int index, double logX);
  int getRegCount() {
    return _regWeights.size();
  }

  double getRegWeight(int index) {
    return _regWeights[index];
  }

  std::string getRegLabel(int index);
  std::string getLabel() {
    return _label;
  }
  void setLabel(std::string label) {
    _label = label;
  }

  virtual ~GridFit() {}
 private:
  void setRegWeight(int index, double value) {
    _regWeights[index] = value;
  }
  static arma::mat fitGridParamsForDataVectorAndWeights(arma::mat D, Arrayd weights, arma::sp_mat P, Array<arma::sp_mat> A);
  static std::shared_ptr<MatExpr> makeDataResidualMatSub(arma::sp_mat P, Array<arma::sp_mat> A, Arrayd weights);
  static arma::sp_mat makeNormalMat(arma::sp_mat P, Array<arma::sp_mat> A, Arrayd weights);
  static std::shared_ptr<MatExpr> makeLsqDataToParamMatSub(arma::sp_mat P, Array<arma::sp_mat> A, Arrayd weights);

  arma::sp_mat _P;
  AutoDiffFunction *_data;
  Array<arma::sp_mat> _regMatrices;
  Array<Arrayb> _splits;

  Arrayd _regWeights;

  std::string _label;
  Array<std::string> _labels;

  // How much THE WHOLE function is weighted
  double _weight;
};

class GridFitter {
 public:
  GridFitter();
  virtual ~GridFitter();

  // GridFitter will allocate a copy of gf on the heap,
  // acquire ownership of this copy and return a pointer to it.
  void add(std::shared_ptr<GridFit> gf);

  // Provide these methods with a vector of initial calibration coefficients
  // and they will optimize it in-place. The vector should be a column of
  // getNLParamCount() elements.
  void solve(arma::mat *XInOut);
  void solveFixedReg(arma::mat *XInOut);

  LevmarSettings settings;

  void setPretuneWeightsIters(int i) {_pretuneWeightsIters = i;}
 private:
  int _pretuneWeightsIters;
  int getNLParamCount();
  std::vector<std::shared_ptr<GridFit> > _terms;

  Arrayi getRegCounts();
  void writeStatus(int i, arma::mat X, int fsize);
};



} /* namespace sail */

#endif /* GRIDFITTER_H_ */
