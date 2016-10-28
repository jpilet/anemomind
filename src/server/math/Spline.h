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

namespace sail {

template <typename T, int PieceCount>
struct SplineBasisFunction {
  static const int pieceCount = PieceCount;
  typedef SplineBasisFunction<T, PieceCount> ThisType;
  typedef Polynomial<T, PieceCount> Piece;

  SplineBasisFunction() {
    if (pieceCount == 1) {
      _pieces[0] = Piece(1.0);
    } else {
      initializeFrom(SplineBasisFunction<T, cmax(1, PieceCount-1)>());
    }
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

private:
  Piece _pieces[PieceCount];
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
  static const int extraBasesPerEndpoint = (Degree + 1)/2;
  static const int coefsPerPoint = 1 + 2*extraBasesPerEndpoint;

  struct Weights {
    int inds[coefsPerPoint];
    T weights[coefsPerPoint];
  };

  RawSplineBasis(int intervalCount) : _intervalCount(intervalCount) {}

  // The interval that the data we are interpolating is spanning
  double lowerDataBound() const {return -0.5;}
  double upperDataBound() const {
    return lowerDataBound() + _intervalCount;
  }

  int coefCount() const {
    return _intervalCount + 2*extraBasesPerEndpoint;
  }

  double basisLocation(int basisIndex) const {
    return -extraBasesPerEndpoint + basisIndex;
  }

  T evaluateBasis(int basisIndex, T loc) const {
    return _basis(loc - basisLocation(basisIndex));
  }

  int computeIntervalIndex(T x) const {
    return std::min(
            std::max(0, int(floor(x - lowerDataBound()))),
              _intervalCount - 1);
  }

  Weights build(T x) const {
    int interval = computeIntervalIndex(x);
    Weights dst;
    for (int i = 0; i < coefsPerPoint; i++) {
      int index = i + interval;
      dst.inds[i] = index;
      dst.weights[i] = evaluateBasis(index, x);
    }
    return dst;
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
private:
  int _intervalCount;
  SplineBasisFunction<T, Degree+1> _basis;
};

template <typename T, int Degree>
class SmoothEndpointSplineBasis {
public:
  static const int excessiveCountPerEndpoint = cmax(0, (Degree+1)/2);

  SmoothEndpointSplineBasis(int intervalCount) :
    _basis(intervalCount) {}

  int coefCount() const {
    return _basis.intervalCount();
  }
private:
  RawSplineBasis<T, Degree> _basis;
};

} /* namespace sail */

#endif /* SERVER_MATH_SPLINE_H_ */
