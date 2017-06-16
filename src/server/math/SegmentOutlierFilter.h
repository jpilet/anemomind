/*
 * SegmentOutlierFilter.h
 *
 *  Created on: 4 Aug 2017
 *      Author: jonas
 */

#ifndef SERVER_MATH_SEGMENTOUTLIERFILTER_H_
#define SERVER_MATH_SEGMENTOUTLIERFILTER_H_

#include <server/math/QuadForm.h>
#include <server/common/logging.h>
#include <array>

namespace sail {

template <int CoordDim>
struct Segment {
  typedef QuadForm<4, CoordDim> QF;

  // 1 to 4 , not higher
  int paramCount = 0;

  // Quadratic form, modelling the cost of this segment as
  // a function of the boundary points
  QF qf;

  // Indices of the parameterization points,
  // starting from left. Only the first 'paramCount'
  // elements are used.
  std::array<int, 4> inds = {0, 0, 0, 0};

  // The "time" coordinates of above points
  std::array<double, 4> X = {0, 0, 0, 0};

  const int* indexBegin() const {return inds;}
  const int* indexEnd() const {return inds + paramCount;}

  int leftMost() const {return inds[0];}
  int rightMost() const {return inds[paramCount-1];}

  typedef Eigen::Matrix<double, CoordDim, 1> Vec;

  static Segment primitive(

      // The index
      int i,

      double x, const Vec& y) {
    Segment dst;
    dst.inds[0] = i;
    dst.paramCount = 1;
    dst.X[0] = x;
    dst.qf = QF::fit(
        std::array<double, 4>{1, 0, 0, 0}.data(),
        y.data());
    return dst;
  }

  double cost() const {
    return (qf + QF::makeReg(1.0e-12)).evaluateOptimalEigenCost();
  }
};


template <int CoordDim>
using QF8 = QuadForm<8, CoordDim>;

template <int CoordDim>
void applyFirstOrderRegAt(
    QF8<CoordDim>* dst, int i, double w) {
  std::array<double, CoordDim> y;
  y.fill(0.0);
  std::array<double, 8> x;
  x.fill(0.0);
  x[i] = -w;
  x[i+1] = w;
  *dst += QF8<CoordDim>::fit(x.data(), y.data());
}

template <int CoordDim>
void applySecondOrderRegAt(
    QF8<CoordDim>* dst,
    const std::array<double, 8>& X,
    int i,
    double w) {
  std::array<double, CoordDim> y;
  y.fill(0.0);
  double k = (X[i+1] - X[i])/(X[i+2] - X[i]);
  std::array<double, 8> x;
  x.fill(0.0);
  x[i] = (1-k)*w;
  x[i+1] = -w;
  x[i+2] = k*w;
  *dst += QF8<CoordDim>::fit(x.data(), y.data());
}

template <typename T, int N>
std::array<T, N + N> merge(
    const std::array<T, N>& A, int na,
    const std::array<T, N>& B, int nb) {
  std::array<T, N + N> dst;
  std::merge(
      A.data(), A.data() + na,
      B.data(), B.data() + nb, dst.data());
  return dst;
}

template <int N, typename T, int M>
std::array<T, N> slice(const std::array<T, M>& src, int from) {
  std::array<T, N> dst;
  for (int i = 0; i < N; i++) {
    dst[i] = src[i + from];
  }
  return dst;
}

template <int CoordDim>
Segment<CoordDim> join(
    const Segment<CoordDim>& left,
    const Segment<CoordDim>& right,
    double regWeight, double difReg) {
  CHECK(left.rightMost() < right.leftMost());
  auto newIndexSet = merge<int, 4>(
      left.inds, left.paramCount,
      right.inds, right.paramCount);
  auto newXSet = merge<double, 4>(
      left.X, left.paramCount,
      right.X, right.paramCount);
  int totalParamCount = left.paramCount + right.paramCount;
  auto shifted = right.qf.template zeroPaddedSlice<8>(-left.paramCount);
  QF8<CoordDim> qf =
        left.qf.template zeroPaddedSlice<8>(0)
      + shifted
      + QF8<CoordDim>::makeReg(1.0e-12);
  applyFirstOrderRegAt(&qf, left.paramCount-1, difReg);
  if (2 <= left.paramCount) {
    applySecondOrderRegAt(
        &qf, newXSet, left.paramCount-2, regWeight);
  }
  if (2 <= right.paramCount) {
    applySecondOrderRegAt(
        &qf, newXSet, left.paramCount-1, regWeight);
  }
  Segment<CoordDim> dst;
  if (totalParamCount <= 4) {
    dst.paramCount = totalParamCount;
    dst.inds = slice<4, int, 8>(newIndexSet, 0);
    dst.X = slice<4, double, 8>(newXSet, 0);
    dst.qf = qf.template zeroPaddedSlice<4>(0);
  } else {
    dst.paramCount = 4;
    auto right = totalParamCount - 2;
    std::array<bool, 8> mask;
    mask.fill(true);
    for (int i = 0; i < (totalParamCount - 4); i++) {
      mask[2 + i] = false;
    }
    for (int i = 0; i < 2; i++) {
      dst.inds[i] = newIndexSet[i];
      dst.X[i] = newXSet[i];
      dst.inds[i + 2] = newIndexSet[right + i];
      dst.X[i + 2] = newXSet[right + i];
    }
    dst.qf = qf.eliminate(mask)
        .template zeroPaddedSlice<4>(0);
  }
  return dst;
}

} /* namespace sail */

#endif /* SERVER_MATH_SEGMENTOUTLIERFILTER_H_ */
