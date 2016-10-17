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
struct SplineBasis {
  typedef Polynomial<T, PieceCount> Piece;

  SplineBasis() {
    for (int i = 0; i < PieceCount; i++) {
      pieces[i] = Piece::zero();
    }
  }

  Piece pieces[PieceCount];

  Piece get(int i) const {
    return 0 <= i && i < PieceCount?
        pieces[i] : Piece::zero();
  }

  static T support() {
    return PieceCount;
  }

  static T leftmost() {
    return -0.5*support();
  }

  static T boundary(int index) {
    return leftmost() + T(index);
  }

  static int pieceIndex(T x) {
    auto diff = x - leftmost();
    return int(std::floor(diff));
  }

  T operator()(T x) const {
    return get(pieceIndex(x)).eval(x);
  }

  void initializeNext(SplineBasis<T, PieceCount+1> *dst) const {
    auto left = Polynomial<T, 2>{-0.5, 1.0};
    auto right = Polynomial<T, 2>{0.5, 1.0};
    auto leftMost = pieces[0].primitive();
    for (int i = 0; i < PieceCount+1; i++) {
      auto split = Polynomial<T, 1>(boundary(i));
      auto leftPolyPrimitive = get(i-1).primitive();
      auto rightPolyPrimitive = get(i).primitive();
      auto leftItg = eval(leftPolyPrimitive, split)
                  - eval(leftPolyPrimitive, left);
      auto rightItg =  eval(rightPolyPrimitive, right)
           - eval(rightPolyPrimitive, split);
      dst->pieces[i] = leftItg + rightItg;

    }
  }

  SplineBasis<T, PieceCount+1> next() const {
    SplineBasis<T, PieceCount+1> dst;
    initializeNext(&dst);
    return dst;
  }
};

} /* namespace sail */

#endif /* SERVER_MATH_SPLINE_H_ */
