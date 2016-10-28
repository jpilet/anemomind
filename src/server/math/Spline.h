/*
 * Spline.h
 *
 *  Created on: 12 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_SPLINE_H_
#define SERVER_MATH_SPLINE_H_

#include <iostream>
#include <server/math/Polynomial.h>
#include <cmath>
#include <server/common/Span.h>
#include <server/common/logging.h>
#include <Eigen/Dense>
#include <map>

namespace sail {

template <typename T, int PieceCount>
struct SplineBasisFunction {
  static const int pieceCount = PieceCount;
  typedef SplineBasisFunction<T, PieceCount> ThisType;
  typedef Polynomial<T, PieceCount> Piece;

  static SplineBasisFunction make() {
    SplineBasisFunction b;
    b.initializeBasis();
    return b;
  }

  Piece getPiece(int i) const {
    return 0 <= i && i < PieceCount?
        _pieces[i] : Piece::zero();
  }

  static T supportWidth() {
    return PieceCount;
  }

  static T rightmost() {
    return 0.5*supportWidth();
  }

  static T leftmost() {
    return -rightmost();
  }

  static T polynomialBoundary(int index) {
    return leftmost() + T(index);
  }

  static int pieceIndex(T x) {
    auto diff = x - leftmost();
    return int(std::floor(diff));
  }

  T operator()(T x) const {
    return getPiece(pieceIndex(x)).eval(x);
  }

  ThisType derivative() const {
    ThisType dst;
    for (int i = 0; i < PieceCount; i++) {
      dst._pieces[i] = _pieces[i].derivative();
    }
    return dst;
  }
private:
  Piece _pieces[PieceCount];

  SplineBasisFunction() {}

  void initializeBasis() {
    if (pieceCount == 1) {
      _pieces[0] = Piece(1.0);
    } else {
      initializeFrom(SplineBasisFunction<T, cmax(1, PieceCount-1)>::make());
    }
  }

  void initializeFrom(const SplineBasisFunction<T, PieceCount-1> &x) {
    auto left = Polynomial<T, 2>{-0.5, 1.0};
    auto right = Polynomial<T, 2>{0.5, 1.0};
    for (int i = 0; i < PieceCount; i++) {
      auto split = Polynomial<T, 1>(x.polynomialBoundary(i));
      auto leftPolyPrimitive = x.getPiece(i-1).primitive();
      auto rightPolyPrimitive = x.getPiece(i).primitive();
      auto leftItg = eval(leftPolyPrimitive, split)
                  - eval(leftPolyPrimitive, left);
      auto rightItg =  eval(rightPolyPrimitive, right)
           - eval(rightPolyPrimitive, split);
      _pieces[i] = leftItg + rightItg;
    }
  }
  void initializeFrom(const ThisType &x) {}
};

template <typename T, int Degree>
class RawSplineBasis {
public:
  typedef SplineBasisFunction<T, Degree+1> SplineType;
  static const int extraBasesPerBoundary = (Degree + 1)/2;
  static const int coefsPerPoint = 1 + 2*extraBasesPerBoundary;

  struct Weights {
    static const int dim = coefsPerPoint;
    int inds[dim];
    T weights[dim];
  };

  RawSplineBasis(int intervalCount) :
    _intervalCount(intervalCount),
    _basisFunction(SplineType::make()) {}

  // The interval that the data we are interpolating is spanning
  double lowerDataBound() const {return -0.5;}
  double upperDataBound() const {
    return lowerDataBound() + _intervalCount;
  }

  int coefCount() const {
    return _intervalCount + 2*extraBasesPerBoundary;
  }

  double basisLocation(int basisIndex) const {
    return -extraBasesPerBoundary + basisIndex;
  }

  T evaluateBasis(const SplineType &basis, int basisIndex, T loc) const {
    return basis(loc - basisLocation(basisIndex));
  }

  T evaluateBasis(int basisIndex, T loc) const {
    return evaluateBasis(_basisFunction, basisIndex, loc);
  }

  int computeIntervalIndex(T x) const {
    return std::min(
            std::max(0, int(floor(x - lowerDataBound()))),
              _intervalCount - 1);
  }

  Weights build(const SplineType &fun, T x) const {
    int interval = computeIntervalIndex(x);
    Weights dst;
    for (int i = 0; i < coefsPerPoint; i++) {
      int index = i + interval;
      dst.inds[i] = index;
      dst.weights[i] = evaluateBasis(fun, index, x);
    }
    return dst;
  }

  Weights build(T x) const {
    return build(_basisFunction, x);
  }

  T evaluate(const T *coefficients, T x) const {
    int offset = computeIntervalIndex(x);
    T sum(0.0);
    for (int i = 0; i < coefsPerPoint; i++) {
      int index = offset + i;
      sum += coefficients[index]*evaluateBasis(index, x);
    }
    return sum;
  }

  int intervalCount() const {return _intervalCount;}

  const SplineType &basisFunction() const {
    return _basisFunction;
  }

  Spani leftmostCoefs() const {
    return Spani(0, coefsPerPoint);
  }

  Spani rightmostCoefs() const {
    int n = coefCount();
    return Spani(n-coefsPerPoint, n);
  }
private:
  int _intervalCount;
  SplineType _basisFunction;
};


class BoundaryIndices {
public:
  BoundaryIndices(Spani left, Spani right, int ep);
  int leftDim() const {return 2*_ep;}
  int totalDim() const;

  bool isLeftIndex(int i) const;
  bool isRightIndex(int i) const;
  bool isInnerIndex(int i) const;

  int computeACol(int i) const;
  int computeBCol(int i) const;
private:
  int epRight() const;
  int innerLimit() const;
  Spani _left, _right;
  int _ep;
};


template <typename T, int Degree>
class SmoothBoundarySplineBasis {
public:
  typedef RawSplineBasis<T, Degree> RawType;
  typedef typename RawType::Weights Weights;

  SmoothBoundarySplineBasis(int intervalCount) :
    _basis(intervalCount) {
    int upperDerivativeBound = Degree+1;
    int lowerDerivativeBound = upperDerivativeBound
        - RawType::extraBasesPerBoundary;
    CHECK(0 < lowerDerivativeBound);
    auto lb = _basis.lowerDataBound();
    auto ub = _basis.upperDataBound();
    auto der = _basis.basisFunction();
    int n = 2*RawType::extraBasesPerBoundary;
    /*Eigen::MatrixXd A(n, n);
    Eigen::MatrixXd B(n, 1);
    int row = 0;
    for (int i = 1; i < upperDerivativeBound; i++) {
      der = der.derivative();
      if (lowerDerivativeBound <= i) {
        auto left = _basis.build(der, lb);
        auto right = _basis.build(der, ub);
        weights.push_back();
        row++;
      }
    }*/
  }

  int coefCount() const {
    return _basis.intervalCount();
  }


private:
  RawSplineBasis<T, Degree> _basis;
};

} /* namespace sail */

#endif /* SERVER_MATH_SPLINE_H_ */
