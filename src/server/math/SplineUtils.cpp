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

namespace sail {

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

TemporalSplineCurve::TemporalSplineCurve(
    Span<TimeStamp> timeSpan,
    Duration<double> period,
    Array<TimeStamp> src,
    Array<double> dst) :
        _offset(timeSpan.minv()), _period(period) {
  CHECK(src.size() == dst.size());
  int sampleCount = int(ceil(toLocal(timeSpan.maxv())));
  _basis = SmoothBoundarySplineBasis<double, 3>(sampleCount);

  auto lhs = SymmetricBandMatrixL<double>::zero(
      sampleCount, decltype(_basis)::Weights::dim);

  auto rhs = MDArray2d(sampleCount, 1);
  rhs.setAll(0.0);
  for (int i = 0; i < sampleCount; i++) {
    lhs.add(i, i, 1.0e-12);
  }
  int n = src.size();
  for (int k = 0; k < n; k++) {
    double x = toLocal(src[k]);
    double y = dst[k];
    auto w = _basis.build(x);
    accumulateNormalEqs(1.0, w, y, &lhs, &rhs);
  }
  Pbsv<double>::apply(&lhs, &rhs);
  _coefs = rhs.getStorage();
}

double TemporalSplineCurve::toLocal(TimeStamp x) const {
  return (x - _offset)/_period;
}

double TemporalSplineCurve::evaluate(TimeStamp t) const {
  return _basis.evaluate(_coefs.ptr(), toLocal(t));
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
  auto x = _mapper.map(t);
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
    MDArray2d *B) {
  auto dst = obs.computeDst(coefs);
  accumulateNormalEqs(
      obs.rejector.computeWeight(),
      obs.weights, Dims,
      dst.data(), A, B);
}

template <int Dims>
void addObservations(
    const MDArray2d &coefs,
    const std::vector<
      typename RobustSplineFit<Dims>::Observation> &obs,
      SymmetricBandMatrixL<double> *A,
      MDArray2d *B) {
  for (auto x: obs) {
    addObservation<Dims>(coefs, x, A, B);
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

  addObservations<Dims>(coefs, observations, &A, &B);

  if (!Pbsv<double>::apply(&A, &B)) {
    LOG(ERROR) << "Failed to solve";
    return MDArray2d();
  }
  return MDArray2d();
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
    x.rejector.update(weight, x.computeResidual(coefs));
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

template class RobustSplineFit<1>;
template class RobustSplineFit<2>;
template class RobustSplineFit<3>;

} /* namespace sail */
