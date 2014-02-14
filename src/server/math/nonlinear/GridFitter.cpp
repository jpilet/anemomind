/*
 * GridFitter.cpp
 *
 *  Created on: 20 janv. 2014
 *      Author: jonas
 */

#include "GridFitter.h"
#include <server/common/ArrayIO.h>
#include <server/common/Uniform.h>
#include <server/common/math.h>
#include <server/common/string.h>
#include <server/math/ADFunction.h>
#include <server/math/armaadolc.h>
#include <server/math/mathutils.h>
#include <server/math/nonlinear/Levmar.h>
#include <server/math/nonlinear/LevmarSettings.h>
#include <server/math/nonlinear/StepMinimizer.h>
#include <server/math/pareto.h>
#include <server/common/logging.h>
#include <server/math/MatExpr.h>

namespace sail {


namespace {
int countRows(Array<arma::sp_mat> A) {
  int counter = 0;
  for (int i = 0; i < A.size(); i++) {
    counter += A[i].n_rows;
  }
  return counter;
}
}


std::shared_ptr<MatExpr> makeDataResidualMat(std::shared_ptr<MatExpr> F,
                              const arma::sp_mat &P, Array<arma::sp_mat> A, Arrayd weights) {
  assert(A.size() == weights.size());
  int indims = F->cols();

  MatExprBuilder builder;
  builder.push(P);
  builder.push(F);
  builder.mul();
  builder.push(-arma::eye(indims, indims));
  builder.add();
  builder.push(arma::sp_mat(weights[0]*A[0]));
  for (int i = 1; i < A.size(); i++) {
    builder.push(arma::sp_mat(weights[i]*A[i]));
    builder.vcat();
  }
  builder.push(F);
  builder.mul();
  builder.vcat();
  assert(builder.single());
  return builder.top();
}

std::shared_ptr<MatExpr> GridFit::makeDataResidualMatSub(arma::sp_mat P, Array<arma::sp_mat> A, Arrayd weights) {
  return makeDataResidualMat(makeLsqDataToParamMatSub(P, A, weights), P, A, weights);
}


arma::sp_mat GridFit::makeNormalMat(arma::sp_mat P, Array<arma::sp_mat> A, Arrayd weights) {
  int count = A.size();
  assert(count == weights.size());
  arma::sp_mat PtP = P.t()*P;
  arma::sp_mat K = PtP;
  for (int i = 0; i < count; i++) {
    arma::sp_mat a = A[i];
    K = K + sqr(weights[i])*(a.t()*a);
  }
  return K;
}

// returns inv(A)*B
// Uses the eigendecomposition of Armadillo for sparse matrices to achieve this.
std::shared_ptr<MatExpr> solveSparseSparseEigsOLD(arma::sp_mat A, std::shared_ptr<MatExpr> B) {
  arma::mat vecs;
  arma::vec vals;
  arma::eigs_sym(vals, vecs, A, A.n_cols);
  arma::sp_mat Dinv = spDiag(invElements(vals)); // A sparse diagonla matrix

  MatExprBuilder builder;
  builder.push(vecs);
  builder.push(Dinv);
  builder.mul();
  builder.push(vecs.t());
  builder.push(B);
  builder.mul();
  builder.mul();
  assert(builder.single());
  return builder.top();
}

std::shared_ptr<MatExpr> solveSparseSparseEigs(arma::sp_mat A, std::shared_ptr<MatExpr> B) {
  arma::mat Adense = arma::inv(MAKEDENSE(A));
  return std::shared_ptr<MatExpr>(new MatExprProduct(std::shared_ptr<MatExpr>(new MatExprDense(Adense)), B));
}

/*
 * What this function does:
 * Precomputes a Z such that Z*D, D being the nonlinearly parameterized data
 * vector, is the optimal grid fit parameters, given, P
 */
std::shared_ptr<MatExpr> GridFit::makeLsqDataToParamMatSub(arma::sp_mat P, Array<arma::sp_mat> A, Arrayd weights) {
  assert(P.n_nonzero > 0); // otherwise singular
  arma::sp_mat K = makeNormalMat(P, A, weights);
  arma::sp_mat Pt = P.t();
  std::shared_ptr<MatExpr> result = solveSparseSparseEigs(K, std::shared_ptr<MatExpr>(new MatExprSparse(Pt)));
  return result;
}



GridFit::GridFit() : _weight(0.0), _data(nullptr) {
}

GridFit::GridFit(arma::sp_mat P, AutoDiffFunction *data, Array<arma::sp_mat> regMatrices, Array<Arrayb> splits,
                 Arrayd regWeights,
                 Array<std::string> regWeightLabels,
                 double weight) : _P(P), _data(data), _regMatrices(regMatrices), _splits(splits),
  _regWeights(regWeights),
  _labels(regWeightLabels),
  _weight(weight) {
  assert(P.n_nonzero > 0);
  assert(splits.hasData());
  assert(_labels.empty() || _labels.size() == _regWeights.size());
  assert(_regWeights.size() == _regMatrices.size());
  assert(_data->outDims() == P.n_rows);
  assert(_data->outDims() == splits[0].size());
}


int GridFit::getNLParamCount() const {
  assert(_data != nullptr);
  return _data->inDims();
}

arma::sp_mat GridFit::makePsel(Arrayb sel) {
  if (sel.hasData()) {
    return makeSpSel(sel)*_P;
  } else {
    return _P;
  }
}


std::shared_ptr<MatExpr> GridFit::makeDataToParamMat(Arrayb sel) {
  arma::sp_mat Psel = makePsel(sel);
  return makeLsqDataToParamMatSub(Psel, _regMatrices, _regWeights);
}

std::shared_ptr<MatExpr> GridFit::makeDataToResidualsMat(Arrayb sel) {
  arma::sp_mat Psel = makePsel(sel);
  std::shared_ptr<MatExpr> R = makeDataResidualMatSub(Psel, _regMatrices, _regWeights);
  int n = R->rows();
  std::shared_ptr<MatExpr> W(new MatExprSparse(_weight*arma::speye(n, n)));
  return std::shared_ptr<MatExpr>(new MatExprProduct(W, R));
}

void GridFit::setExpRegWeight(int index, double logX) {
  setRegWeight(index, exp(logX));
}

std::string GridFit::getRegLabel(int index) {
  return (_labels.empty()? stringFormat("regw(%d/%d)", index+1, _regWeights.size()) : _labels[index]);
}

std::shared_ptr<MatExpr> GridFit::makeCrossValidationFitnessMat() {
  SCOPEDLOG(INFO, "Build crossvalidation matrix");
  // true  <=> test
  // false <=> train

  int testCount = 0;
  int splitCount = _splits.size();
  for (int i = 0; i < splitCount; i++) {
    testCount += countTrue(_splits[i]);
  }

  MatExprBuilder builder;

  int offset = 0;
  for (int i = 0; i < splitCount; i++) {
    SCOPEDLOG(INFO, stringFormat("Split %d/%d", i+1, splitCount));
    Arrayb test = _splits[i];
    Arrayb train = neg(test);
    arma::sp_mat selTrain = makeSpSel(train);
    arma::sp_mat Ptrain = selTrain*_P;
    assert(_P.n_nonzero > 0);
    assert(Ptrain.n_nonzero > 0);
    std::shared_ptr<MatExpr> fit = makeLsqDataToParamMatSub(Ptrain, _regMatrices, _regWeights);

    arma::sp_mat selTest = makeSpSel(test);
    arma::sp_mat Ptest = selTest*_P;
    int ptrows = Ptest.n_rows;
    int next = offset + ptrows;

    //std::shared_ptr<MatExpr> FS(new MatExprProduct(fit, selTrain));
    //M.rows(offset, next-1) = selTest*(_P*FS - arma::eye(_P.n_rows, _P.n_rows));
      builder.push(selTest);
      builder.push(_P);
      builder.push(fit);
      builder.push(selTrain);
      builder.mul();
      builder.mul();
      builder.push(arma::sp_mat(-1.0*arma::speye(_P.n_rows, _P.n_rows)));
      builder.add();
      builder.mul();
    if (i == 0) {
      assert(builder.single());
    } else {
      assert(builder.size() == 2);
      builder.vcat();
    }
  }
  assert(builder.single());
  return builder.top();
}

arma::mat GridFit::fitGridParamsForDataVectorAndWeights(arma::mat D, Arrayd weights, arma::sp_mat P, Array<arma::sp_mat> A) {
  std::shared_ptr<MatExpr> Pt(new MatExprSparse(P.t()));
  std::shared_ptr<MatExpr> K = solveSparseSparseEigs(GridFit::makeNormalMat(P, A, weights), Pt);
  return K->mulWithDense(D);
}

double GridFit::evalObjfForDataVector(arma::mat D) {
  assert(D.n_rows == _data->outDims());
  assert(D.n_cols == 1);
  arma::mat params = fitGridParamsForDataVectorAndWeights(D, _regWeights, _P, _regMatrices);

  double cost = SQNORM(_P*params - D);
  int count = _regMatrices.size();
  for (int i = 0; i < count; i++) {
    arma::mat R = _regWeights[i]*_regMatrices[i]*params;
    cost += SQNORM(R);
  }

  return sqr(_weight)*cost;
}

double GridFit::evalCrossValidationFitness(arma::mat D) {
  return SQNORM(makeCrossValidationFitnessMat()->mulWithDense(D)); // We may want to do something more efficient here, in future.
}

GridFitter::GridFitter() {
  settings.verbosity = 0;
  _pretuneWeightsIters = 0;
}



GridFitter::~GridFitter() {
}

void GridFitter::add(std::shared_ptr<GridFit> gf) {
  if (!_terms.empty()) {
    assert(gf->getNLParamCount() == getNLParamCount());
  }
  _terms.push_back(gf);
}


namespace {
// This class just adds some abstraction to the inner workings
// of GridFitter::solve. It is not intended to be used by anyone else.
//
// Evaluates the objective function given the
// nonlinear parameter vector. Regularization weights
// are assumed to remain constant throughout the lifetime of
// this object.
class GridFitPlayer1 : public Function {
 public:
  GridFitPlayer1(ParetoFrontier &frontier, std::vector<std::shared_ptr<GridFit> > &fits);

  int inDims() {
    return _inDims;
  }
  int outDims() {
    return _outDims;
  }
  void eval(double *Xin, double *Fout, double *Jout);

  bool acceptor(double *Xin, double objfVal);
 private:
  ParetoFrontier &_frontier;

  std::vector<std::shared_ptr<GridFit> > &_fits; // The GridFit's

  // Matrices used to evaluate the main objective function (used by evalAD)
  Array<std::shared_ptr<MatExpr> > _Rmats;

  // Matrices used to compute cross validation.
  Array<std::shared_ptr<MatExpr> > _cvmats;

  int _outDims, _inDims, _maxDataLen;

  // Evaluates the cross validation score for every GridFit given the nonlinear parameter vector
  void evalCrossValidations(double *Xin, double *cvOut);
};

GridFitPlayer1::GridFitPlayer1(ParetoFrontier &frontier, std::vector<std::shared_ptr<GridFit> > &fits) :
  _frontier(frontier), _fits(fits) {
  int count = fits.size();
  _Rmats.create(count);
  _cvmats.create(count);
  _inDims = fits[0]->getData().inDims();
  _outDims = 0;
  _maxDataLen = 0;
  for (int i = 0; i < count; i++) {
    GridFit *fit = fits[i].get();
    assert(fit->getData().inDims() == _inDims);
    std::shared_ptr<MatExpr> R = fit->makeDataToResidualsMat();
    _Rmats[i] = R;
    _cvmats[i] = fit->makeCrossValidationFitnessMat();
    _outDims += R->rows();
    _maxDataLen = std::max(_maxDataLen, fit->getData().outDims());
  }
}

void GridFitPlayer1::eval(double *Xin, double *Fout, double *Jout) {
  bool outputJ = Jout != nullptr;
  int count = _fits.size();
  int offset = 0;
  Array<double> tempF(_maxDataLen), tempJ(_maxDataLen*_fits[0]->getNLParamCount());
  arma::mat Jdst(Jout, outDims(), inDims(), false, true);
  for (int i = 0; i < count; i++) {
    std::shared_ptr<MatExpr> R = _Rmats[i];
    double *Fouti = Fout + offset;
    arma::mat dst(Fouti, R->rows(), 1, false, true);
    AutoDiffFunction &data = _fits[i]->getData();

    int od = data.outDims();
    arma::mat D(tempF.getData(), od, 1, false, true);
    arma::mat JD(tempJ.getData(), od, data.inDims(), false, true);

    data.eval(Xin, tempF.getData(), (outputJ? tempJ.getData() : nullptr));

    dst = R->mulWithDense(D);
    if (outputJ) {
      Jdst.rows(offset, offset + R->rows() - 1) = R->mulWithDense(JD);
    }

    offset += R->rows();
  }
  assert(offset == _outDims);
}

bool GridFitPlayer1::acceptor(double *Xin, double objfVal) {
  int count = _fits.size();
  Arrayd costs(1 + count);
  evalCrossValidations(Xin, costs.ptr(1));
  return _frontier.insert(ParetoElement(costs));
}

void GridFitPlayer1::evalCrossValidations(double *Xin, double *cvOut) {
  int count = _fits.size();
  arma::mat X(Xin, inDims(), 1, false, true);

  Arrayd temp(_maxDataLen);
  for (int i = 0; i < count; i++) {
    Function &fun = _fits[i]->getData();
    arma::mat D(temp.getData(), fun.outDims(), 1, false, true);
    fun.eval(Xin, temp.getData());
    cvOut[i] = SQNORM(_cvmats[i]->mulWithDense(D));
  }
}






// This class adds abstraction to the inner workings of GridFitter::solve
// It should not be used by anyone else.
class GridFitOtherPlayers {
 public:
  GridFitOtherPlayers(ParetoFrontier &frontier, std::vector<std::shared_ptr<GridFit> > &fits, arma::mat X);
  void optimize(Array<Arrayd> stepSizes);
 private:
  ParetoFrontier &_frontier;
  Array<arma::mat> _D;
  std::vector<std::shared_ptr<GridFit> > &_fits;

  void optimizeForGridFit(int index, Arrayd stepSizes);
  double evalObjf();
  ParetoElement makeParetoElementVector();
};

GridFitOtherPlayers::GridFitOtherPlayers(ParetoFrontier &frontier,
    std::vector<std::shared_ptr<GridFit> > &fits, arma::mat X) : _frontier(frontier), _fits(fits) {
  int count = _fits.size();
  _D.create(count);
  for (int i = 0; i < count; i++) {
    Function &fun = _fits[i]->getData();
    _D[i] = arma::mat(fun.outDims(), 1);
    fun.eval(X.memptr(), _D[i].memptr());
  }
}

void GridFitOtherPlayers::optimize(Array<Arrayd> stepSizes) {
  int count = _fits.size();
  for (int i = 0; i < count; i++) {
    SCOPEDLOG(INFO, stringFormat("Tuning reg weights for gridfit %d/%d (%s)",
        i+1, count, _fits[i]->getLabel().c_str()));
    optimizeForGridFit(i, stepSizes[i]);
  }
}

void GridFitOtherPlayers::optimizeForGridFit(int index, Arrayd stepSizes) {
  GridFit *f = _fits[index].get();
  const arma::mat &dataVector = _D[index];
  int rc = f->getRegCount();
  for (int i = 0; i < rc; i++) {
    SCOPEDLOG(INFO, stringFormat("Tuning reg weight %d/%d (%s)",
        i+1, rc, f->getRegLabel(i).c_str()));
    // It is best to do this search in the logarithmic domain,
    // because this way, the weight stays positive.
    double initReg = log(f->getRegWeight(i));

    double initStep = stepSizes[i];
    auto objf = [&] (double x) {
      SCOPEDLOG(INFO, stringFormat("Evaluate cross-validation fitness at %.3g", exp(x)));
      f->setExpRegWeight(i, x);
      double fitness = f->evalCrossValidationFitness(dataVector);
      return fitness;
    };
    auto acceptor = [&] (double x, double val) {
      f->setExpRegWeight(i, x);
      return _frontier.insert(makeParetoElementVector());
    };
    StepMinimizerState initState(initReg, initStep, objf(initReg));
    StepMinimizer minimizer;
    minimizer.setAcceptor(acceptor);
    StepMinimizerState finalState = minimizer.takeStep(initState, objf);
    stepSizes[i] = finalState.getStep();
    f->setExpRegWeight(i, finalState.getX());
  }
}

double GridFitOtherPlayers::evalObjf() {
  int count = _fits.size();
  double value = 0.0;
  for (int i = 0; i < count; i++) {
    value += _fits[i]->evalObjfForDataVector(_D[i]);
  }
  return value;
}

ParetoElement GridFitOtherPlayers::makeParetoElementVector() {
  int count = _fits.size();
  Arrayd values(1 + count);
  values[0] = evalObjf();
  for (int i = 0; i < count; i++) {
    values[1 + i] = _fits[i]->evalCrossValidationFitness(_D[i]);
  }
  return ParetoElement(values);
}

Array<Arrayd> initStepSizes(Arrayi regCounts, double initStepSize) {
  return regCounts.map<Arrayd>([=] (int n) {
    Arrayd dst(n);
    dst.setTo(initStepSize);
    return dst;
  });
}
}

void GridFitter::writeStatus(int i, arma::mat X, int fsize) {
  // Status output for this iteration
  if (i == 0) {
    cout << "################################################ GRIDFITTER BEGIN SOLVE" << endl;
  }


  if (i == -1) {
    cout << "######################## DONE SOLVING IT" << endl;
  } else {
    std::cout << "\n\n\n### GRIDFITTER ITERATION " << i+1 << std::endl;
  }

  std::cout << "   X = " << X.t() << endl;
  int gfCount = _terms.size();
  for (int i = 0; i < gfCount; i++) {
    std::shared_ptr<GridFit> gf = _terms[i];
    std::string l = gf->getLabel();
    std::cout << "   " << (l.empty()? stringFormat("GridFit(%d/%d)", i+1, gfCount) : l) << std::endl;
    int rCount = gf->getRegCount();
    for (int j = 0; j < rCount; j++) {
      std::cout << "     " << gf->getRegLabel(j) << " = " << gf->getRegWeight(j) << std::endl;
    }
  }
  std::cout << "   Frontier size: " << fsize << endl;
  if (i == -1) {
    cout << "########################################################################################" << endl;
  }
}

void GridFitter::solve(arma::mat *XInOut) {
  arma::mat &X = *(XInOut);
  assert(X.size() == getNLParamCount());

  const double initStepSize = 0.1;
  Array<Arrayd> stepSizes = initStepSizes(getRegCounts(), initStepSize);

  ParetoFrontier frontier;

  const int iters = 30;
  for (int i = 0; i < iters; i++) {
    writeStatus(i, X, frontier.size());

    // Part 1: Optimize Player 1 (the objective function)
    if (_pretuneWeightsIters <= i) {
      LOG(INFO) << "    PART 1: Adjusting calibration parameters...";
      LOG(INFO) << "    Instantiate player 1";
      GridFitPlayer1 objf(frontier, _terms);
      LOG(INFO) << "    Instantiated.";
      settings.acceptor = [&] (double *Xd, double val) {
        return objf.acceptor(Xd, val);
      };
      LevmarState lmState(X);
      LOG(INFO) << "    Take a step";
      lmState.step(settings, objf);
      LOG(INFO) << "    Done taking a step.";
      X = lmState.getX();
    } else {
      LOG(INFO) << "    PART 1: Skipped, pretuning weights.";
    }

    // Part 2: Adjust the regularization weights of every grid fit.
    {
      LOG(INFO) << "    PART 2: Adjusting regularization weights...";
      GridFitOtherPlayers other(frontier, _terms, X);
      other.optimize(stepSizes);
    }
  }
  writeStatus(-1, X, frontier.size());
}

void GridFitter::solveFixedReg(arma::mat *XInOut) {
  arma::mat &X = *XInOut;
  LevmarState lmState(X);
  ParetoFrontier frontier;
  GridFitPlayer1 objf(frontier, _terms);
  lmState.minimize(settings, objf);

  X = lmState.getX();
}

int GridFitter::getNLParamCount() {
  if (_terms.empty()) {
    return -1;
  } else {
    return _terms.front()->getNLParamCount();
  }
}

Arrayi GridFitter::getRegCounts() {
  int count = _terms.size();
  Arrayi counts(count);
  for (int i = 0; i < count; i++) {
    counts[i] = _terms[i]->getRegCount();
  }
  return counts;
}

Arrayb makeRandomSplit(int count) {
  assert(count >= 2);
  Uniform rng(0.0, 1.0);
  Arrayb split(count);
  int trueCount = 0;
  for (int i = 0; i < count; i++) {
    bool incl = rng.gen() > 0.5;
    split[i] = incl;
    trueCount += (incl? 1 : 0);
  }
  if (trueCount == 0 || trueCount == count) { // <-- If this condition is satisfied, the
                                              //     resulting split could result in sin-
                                              //     gular matrices. Therefore, find a
                                              //     new split.

    return makeRandomSplit(count); // <-- Infinite recursion will not happen: The probability that
                                   //     we will have found a valid split after N tries tends
                                   //     to 1 as N tends towards infinity. However, we require count >= 2.
  }
  return split;
}

Array<Arrayb> makeRandomSplits(int numSplits, int size) {
  Array<Arrayb> dst(numSplits);
  for (int i = 0; i < numSplits; i++) {
    dst[i] = makeRandomSplit(size);
  }
  return dst;
}



} /* namespace sail */
