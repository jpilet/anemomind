/*
 * SplineUtils.cpp
 *
 *  Created on: 28 Oct 2016
 *      Author: jonas
 */

#include <server/math/SplineUtils.h>
#include <server/math/lapack/BandWrappers.h>
#include <server/common/ArrayIO.h>
#include <server/common/LineKM.h>
#include <server/common/indexed.h>
#include <server/common/TimedValue.h>
#include <server/nautical/common.h>

namespace sail {

template <int N>
Eigen::Matrix<double, N, 1> getVec(
    const MDArray2d &src, int i) {
  Eigen::Matrix<double, N, 1> dst;
  for (int i = 0; i < N; i++) {
    dst(i) = src(i, 0);
  }
  return dst;
}

std::ostream &operator<<(std::ostream &s,
    SmoothBoundarySplineBasis<double, 3>::Weights w) {
  s << "Weights(";
  for (int i = 0; i < w.dim; i++) {
    s << " i=" << w.inds[i] << " w=" << w.weights[i] << ", ";
  }
  s << ")";
  return s;
}

void accumulateNormalEqs(
    double weight,
    const SmoothBoundarySplineBasis<double, 3>::Weights &w,
    int dim,
    const double *y,
    SymmetricBandMatrixL<double> *lhs,
    MDArray2d *rhs) {
  double w2 = sqr(weight);
  for (int i = 0; i < w.dim; i++) {
    if (w.isSet(i)) {
      for (int j = 0; j < w.dim; j++) {
        if (w.isSet(j)) {
          int I = w.inds[i];
          int J = w.inds[j];
          lhs->add(I, J, w2*w.weights[i]*w.weights[j]);
        }
      }
      for (int j = 0; j < dim; j++) {
        (*rhs)(w.inds[i], j) += w2*w.weights[i]*y[j];
      }
    }
  }
}

void accumulateNormalEqs(
    double weight,
    const SmoothBoundarySplineBasis<double, 3>::Weights &w,
    double y,
    SymmetricBandMatrixL<double> *lhs,
    MDArray2d *rhs) {
  accumulateNormalEqs(weight, w, 1, &y, lhs, rhs);
}

Arrayd fitSplineCoefs(
    const SmoothBoundarySplineBasis<double, 3> &basis,
    std::function<double(int)> sampleFun) {
  int n = basis.coefCount();
  auto lhs = SymmetricBandMatrixL<double>::zero(
      n, SmoothBoundarySplineBasis<double, 3>::Weights::dim);
  auto rhs = MDArray2d(n, 1);
  for (int i = 0; i < n; i++) {
    accumulateNormalEqs(1.0, basis.build(i), sampleFun(i), &lhs, &rhs);
  }
  Pbsv<double>::apply(&lhs, &rhs);
  return rhs.getStorage();
}


void SplineFittingProblem::initialize(int n, int dim, double k) {
  _A = SymmetricBandMatrixL<double>::zero(
                         n,
                         Basis::Weights::dim);
  _B = MDArray2d(n, dim);
  _B.setAll(0.0);
  _bases = SmoothBoundarySplineBasis<double, 3>(n).makeDerivatives();
  _factors = makePowers(n, k);
}

Array<double> makePowers(int n, double f) {
  Array<double> dst = Array<double>::fill(n, 1.0);
  dst[0] = 1.0;
  for (int i = 1; i < n; i++) {
    dst[i] = f*dst[i-1];
  }
  return dst;
}

SplineFittingProblem::SplineFittingProblem(
    const TimeMapper &mapper, int dim) : _mapper(mapper) {
  initialize(mapper.sampleCount, dim, 1.0/_mapper.period.seconds());
}

SplineFittingProblem::SplineFittingProblem(int n, int dim) {
  initialize(n, dim, 1.0);
}

void SplineFittingProblem::addCost(int order,
    double weight,
    double x, double *y) {
  auto w = _bases[order].build(x);
  accumulateNormalEqs(weight, w, _B.cols(), y, &_A, &_B);
}

void SplineFittingProblem::addRegularization(int order, double weight) {
  auto y = Array<double>::fill(_B.cols(), 0.0);
  for (int i = 0; i < _mapper.sampleCount; i++) {
    addCost(order, weight, i, y.ptr());
  }
}

MDArray2d SplineFittingProblem::solve() {

  MDArray2d result = _B;
  if (Pbsv<double>::apply(&_A, &_B)) {
    _A = SymmetricBandMatrixL<double>();
    _B = MDArray2d();
    return result;
  }
  LOG(ERROR) << "Failed to solve spline fitting problem";
  return MDArray2d();
}

void SplineFittingProblem::addCost(
    int order,
    double weight,
    TimeStamp t, double *y) {
  double x = _mapper.mapToReal(t);
  addCost(order, weight*_factors[order],
      x, y);
}

SplineFittingProblem::Basis SplineFittingProblem::basis(int i) const {
  return _bases[i];
}

void SplineFittingProblem::disp() const {
  std:cout << "Spline fitting problem:\n";
  std::cout << " A = \n" << _A.makeDense() << std::endl;
  std::cout << " B = \n" << _B << std::endl;
}

MDArray2d computeSplineCoefs(const MDArray2d &splineSamples) {
  int rows = splineSamples.rows();
  int cols = splineSamples.cols();
  SplineFittingProblem problem(
      rows, splineSamples.cols());
  int dim = splineSamples.cols();
  auto tmp = Array<double>::fill(cols, 0.0);
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      tmp[j] = splineSamples(i, j);
    }
    problem.addCost(0, 1.0, i, tmp.ptr());
  }
  auto dst = problem.solve();
  return dst;
}

template <int Dims>
RobustSplineFit<Dims>::RobustSplineFit(const TimeMapper &mapper,
    const Settings &settings)
  : _mapper(mapper),
    _factors(makePowers(
      mapper.sampleCount, mapper.period.seconds())),
      _bases(Basis(mapper.sampleCount).makeDerivatives()),
      _settings(settings) {}

template <int Dims>
void RobustSplineFit<Dims>::addObservation(TimeStamp t,
    int order,
    const Vec &value,
    double sigma,
    const VecFun &valueFun) {
  auto x = _mapper.mapToReal(t);
  auto weights = _bases[order].build(x);
  double dstScale = _factors[order];
  OutlierRejector::Settings os;
  os.initialAlpha = _settings.minWeight;
  os.initialBeta = _settings.minWeight;
  os.sigma = sigma;
  _observations.push_back(Observation(
      _bases[0].build(x),
      weights,
      dstScale, value,
      valueFun, os));
}

void applyRegularization(
    SymmetricBandMatrixL<double> *A,
    MDArray2d *B,
    const SmoothBoundarySplineBasis<double, 3> &basis,
    double w) {
  auto z = Arrayd::fill(B->cols(), 0.0);
  int n = B->rows();
  for (int i = 0; i < n; i++) {
    auto weights = basis.build(i);
    accumulateNormalEqs(
        w, weights, z.size(),
        z.ptr(), A, B);
  }
}

template <int Dims>
void addObservation(
    const MDArray2d &coefs,
    const typename RobustSplineFit<Dims>::Observation &obs,
    SymmetricBandMatrixL<double> *A,
    MDArray2d *B, bool ignoreConstantIfVariable) {
  if (coefs.empty() && ignoreConstantIfVariable && bool(obs.dstFun)) {
    return;
  }
  auto weight = obs.rejector.computeWeight();
  auto dst = obs.computeDst(coefs);

  accumulateNormalEqs(
      weight,
      obs.weights, Dims,
      dst.data(), A, B);
}

template <int Dims>
void addObservations(
    const MDArray2d &coefs,
    const std::vector<
      typename RobustSplineFit<Dims>::Observation> &obs,
      SymmetricBandMatrixL<double> *A,
      MDArray2d *B, bool ignoreConstantIfVariable) {
  for (auto x: obs) {
    addObservation<Dims>(coefs, x, A, B,
        ignoreConstantIfVariable);
  }
}

template <int Dims>
MDArray2d solveForObservations(
    const MDArray2d &coefs,
    int n,
    const Array<SmoothBoundarySplineBasis<double, 3>> &bases,
    const typename RobustSplineFit<Dims>::Settings
      &settings,
    const std::vector<typename RobustSplineFit<Dims>::Observation>
      &observations) {
  auto A = SymmetricBandMatrixL<double>::zero(
      n, RobustSplineFit<Dims>::Weights::dim);
  auto B = MDArray2d(n, Dims);
  B.setAll(0.0);
  applyRegularization(&A, &B,
      bases[settings.wellPosednessOrder],
      settings.wellPosednessReg);
  applyRegularization(&A, &B,
      bases[settings.regOrder],
      settings.regWeight);

  addObservations<Dims>(coefs, observations, &A, &B,
      settings.ignoreConstantIfVariable);

  if (!Pbsv<double>::apply(&A, &B)) {
    LOG(ERROR) << "Failed to solve";
    return MDArray2d();
  }
  return B;
}

template <int Dims>
Eigen::Matrix<double, Dims, 1> evaluateSpline(
    const SmoothBoundarySplineBasis<double, 3>::Weights &weights,
        const MDArray2d &coefs) {
  assert(Dims == coefs.cols());
  Eigen::Matrix<double, Dims, 1> dst;
  for (int i = 0; i < Dims; i++) {
    dst(i) = weights.evaluate(coefs.getPtrAt(0, i));
  }
  return dst;
}

template Eigen::Matrix<double, 1, 1> evaluateSpline<1>(
    const SmoothBoundarySplineBasis<double, 3>::Weights &weights,
        const MDArray2d &coefs);
template Eigen::Matrix<double, 2, 1> evaluateSpline<2>(
    const SmoothBoundarySplineBasis<double, 3>::Weights &weights,
        const MDArray2d &coefs);
template Eigen::Matrix<double, 3, 1> evaluateSpline<3>(
    const SmoothBoundarySplineBasis<double, 3>::Weights &weights,
        const MDArray2d &coefs);

template <int Dims>
void updateWeights(double weight,
    const MDArray2d &coefs,
    std::vector<typename RobustSplineFit<Dims>::Observation> *obs) {
  for (auto &x: *obs) {
    auto residual = x.computeResidual(coefs);
    x.rejector.update(weight, residual);
  }
}

template <int Dims>
MDArray2d RobustSplineFit<Dims>::solve() {
  LineKM logWeights(0, _settings.iters-1,
      log(_settings.minWeight),
      log(_settings.maxWeight));
  MDArray2d coefs;
  for (int i = 0; i < _settings.iters; i++) {
    coefs = solveForObservations<Dims>(
        coefs,
        _mapper.sampleCount, _bases,
        _settings, _observations);
    updateWeights<Dims>(exp(logWeights(i+1)), coefs,
        &_observations);
  }
  return coefs;
}

template <int Dims>
SmoothBoundarySplineBasis<double, 3>
  RobustSplineFit<Dims>::basis(int i) const {
  return _bases[i];
}

Arrayi initializeIndices(int n) {
  Arrayi dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = i;
  }
  return dst;
}

MDArray2d initializeCoefs(int coefCount, int dim) {
  MDArray2d dst = MDArray2d(coefCount, dim);
  dst.setAll(0.0);
  return dst;
}


template <int N>
void addObservations(
    const CubicBasis &basis,
    SymmetricBandMatrixL<double> *lhs,
      MDArray2d *rhs, const Arrayi &inds,
      const Array<VecObs<N>> &observations) {
  for (auto i: inds) {
    auto obs = observations[i];
    auto w = basis.build(obs.first);
    accumulateNormalEqs(
        1.0, w,
        N, obs.second.data(), lhs, rhs);
  }
}

template <int N>
void applyStepRegularization(
    SymmetricBandMatrixL<double> *lhs,
    MDArray2d *rhs,
    const CubicBasis &basis,
    double weight,
    const MDArray2d &coefs) {
  int n = basis.coefCount();
  CHECK(n == coefs.rows());
  for (int i = 0; i < n; i++) {
    auto w = basis.build(i);
    auto y = evaluateSpline<N>(w, coefs);
    accumulateNormalEqs(
        weight, w,
        N, y.data(), lhs, rhs);
  }
}

template <int N>
MDArray2d iterateAutoRegOne(
    const Array<CubicBasis> &bases,
    const MDArray2d &coefs,
    const Array<VecObs<N>> &observations,
    const Array<int> &inds,
    const AutoRegSettings &settings) {
  int n = coefs.rows();
  auto lhs = SymmetricBandMatrixL<double>::zero(
      n, CubicBasis::Weights::dim);
  auto rhs = MDArray2d(n, N);
  rhs.setAll(0.0);
  addObservations<N>(bases[0], &lhs, &rhs, inds, observations);
  applyStepRegularization<N>(
        &lhs, &rhs, bases[settings.order], settings.weight,
        coefs);
  applyRegularization(
        &lhs, &rhs, bases[0], settings.wellPosednessReg);
  if (!Pbsv<double>::apply(&lhs, &rhs)) {
    return MDArray2d();
  }
  return rhs;
}

Array<Arrayi> performSplit(const Array<int> &inds) {
  int m = inds.middle();
  return Array<Arrayi>{
    inds, inds.sliceTo(m), inds.sliceFrom(m)
  };
}

template <int N>
Array<MDArray2d> iterateAutoReg(
    const Array<CubicBasis> &bases,
    const MDArray2d &coefs,
    const Array<VecObs<N>> &observations,
    const Array<Arrayi> &inds,
    const AutoRegSettings &settings) {
  Array<MDArray2d> dst(inds.size());
  for (auto k : indexed(inds)) {
    auto K = iterateAutoRegOne<N>(
        bases, coefs, observations,
        k.second, settings);
    if (K.empty()) {
      return Array<MDArray2d>();
    }
    dst[k.first] = K;
  }
  return dst;
}


template <int N>
double evaluateData(
    const CubicBasis &basis,
    const Array<VecObs<N>> &observations,
    const Array<int> inds,
    const MDArray2d &coefs) {
  double sum = 0.0;
  for (auto i: inds) {
    auto obs = observations[i];
    auto w = basis.build(obs.first);
    sum += (evaluateSpline<N>(w, coefs) - obs.second)
        .squaredNorm();
  }
  return sum;
}

template <int N>
double evaluateCrossValidationCost(
    const CubicBasis &basis,
    const Array<VecObs<N>> &observations,
    const Array<MDArray2d> &coefs,
    const Array<Array<int>> &splitInds) {
  CHECK(coefs.size() == 3);
  CHECK(splitInds.size() == 3);
  return evaluateData<N>(
              basis, observations,
              splitInds[1], coefs[2])
       + evaluateData<N>(
              basis, observations,
              splitInds[2], coefs[1]);
}

template <int N>
AutoRegResults fitSplineAutoReg(
    int coefCount,
    const Array<VecObs<N>>
      &observations,
      const AutoRegSettings &settings,
      RNG *rng) {
  if (coefCount < 1) {
    LOG(ERROR) << "Too few coefficients";
    return AutoRegResults();
  }
  if (observations.size() < 2) {
    LOG(ERROR) << "Too few observations";
    return AutoRegResults();
  }

  Arrayi inds = initializeIndices(observations.size());
  auto coefs = initializeCoefs(coefCount, N);
  auto bases = CubicBasis(coefCount).makeDerivatives();

  double lastCrossValidationCost = std::numeric_limits<
      double>::infinity();
  ArrayBuilder<double> costs;
  for (int i = 0; i < settings.maxIters; i++) {
    std::shuffle(inds.begin(), inds.end(), *rng);
    auto splitInds = performSplit(inds);
    auto nextCoefs = iterateAutoReg<N>(
        bases, coefs, observations,
        splitInds, settings);
    if (nextCoefs.empty()) {
      LOG(ERROR) << "Failed";
      return AutoRegResults();
    }

    double crossValidationCost =
        evaluateCrossValidationCost<N>(bases[0],
            observations, nextCoefs, splitInds);
    costs.add(crossValidationCost);

    if (lastCrossValidationCost <= crossValidationCost) {
      break;
    } else {
      coefs = nextCoefs[0];
      lastCrossValidationCost = crossValidationCost;
    }
  }
  return AutoRegResults{costs.get(), coefs};
}

template AutoRegResults fitSplineAutoReg<1>(
    int coefCount,
    const Array<VecObs<1>>
      &observations,
      const AutoRegSettings &settings,
      RNG *rng);
template AutoRegResults fitSplineAutoReg<2>(
    int coefCount,
    const Array<VecObs<2>>
      &observations,
      const AutoRegSettings &settings,
      RNG *rng);


template <typename OpType>
TypedSpline<OpType>::TypedSpline(
    const TimeMapper &mapper,
    const MDArray2d &coefs,
    OpType op) : _timeMapper(mapper),
    _basis(mapper.sampleCount), _coefs(coefs), _op(op) {}


template <typename OpType>
typename OpType::OutputType
  TypedSpline<OpType>::evaluate(const TimeStamp &t) const {
  auto x = _timeMapper.mapToReal(t);
  std::cout << "x = " << x << std::endl;
  auto weights = _basis.build(x);
  auto vec = evaluateSpline<OpType::coefDim>(weights, _coefs);
  return _op.apply(vec);
}

template <typename OpType>
TimeStamp TypedSpline<OpType>::lower() const {
  return _timeMapper.unmap(_basis.raw().lowerDataBound());
}

template <typename OpType>
const TimeMapper &TypedSpline<OpType>::timeMapper() const {
  return _timeMapper;
}

template <typename OpType>
TimeStamp TypedSpline<OpType>::upper() const {
  return _timeMapper.unmap(_basis.raw().upperDataBound());
}

template <typename OpType>
Duration<double> TypedSpline<OpType>::duration() const {
  return upper() - lower();
}

VecObs<2> makeAngleUVecObs(
    const TimeMapper &mapper,
    const TimedValue<Angle<double>> &v) {
  return VecObs<2>(mapper.map(v.time),
      makeNauticalUnitVector<double>(v.value));
}

TypedSpline<UnitVecSplineOp>
  fitAngleSpline(
      const TimeMapper &mapper,
      const Array<TimedValue<Angle<double>>> &angles,
      const AutoRegSettings &settings,
      RNG *rng) {
  Array<VecObs<2>> vecObs(angles.size());
  for (int i = 0; i < angles.size(); i++) {
    vecObs[i] = makeAngleUVecObs(mapper, angles[i]);
  }
  auto fitting = fitSplineAutoReg<2>(
      mapper.sampleCount,
      vecObs, settings, rng);
  if (!fitting.defined()) {
    LOG(ERROR) << "Fitting failed.";
    return TypedSpline<UnitVecSplineOp>();
  }
  return TypedSpline<UnitVecSplineOp>(
      mapper, fitting.coefs, UnitVecSplineOp());
}

template class TypedSpline<UnitVecSplineOp>;
template class RobustSplineFit<1>;
template class RobustSplineFit<2>;
template class RobustSplineFit<3>;

} /* namespace sail */
