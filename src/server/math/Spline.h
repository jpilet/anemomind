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
class SplineBasis {
public:
  static const int coefsPerInterval = cmax(1, 2*Degree);

  struct Weights {
    int inds[2*Degree + 1];
  };

  SplineBasis(int intervalCount) : _intervalCount(intervalCount) {}
  double leftMost() const {return 0.0;}
  double rightMost() const {return _intervalCount;}

  int leftMostCoefIndex() const {
    return int(ceil(-0.5 - Degree));
  }

  int rightMostCoefIndex() const {
    return int(floor(rightMost() + 0.5 + Degree));
  }

  int coefCount() const {
    return 1 + rightMostCoefIndex() - leftMostCoefIndex();
  }
private:
  int _intervalCount;
  SplineBasisFunction<T, Degree+1> _basis;
};

} /* namespace sail */

#endif /* SERVER_MATH_SPLINE_H_ */
