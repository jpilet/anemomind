/*
 * Spline.h
 *
 *  Created on: 12 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_SPLINE_H_
#define SERVER_MATH_SPLINE_H_

#include <server/math/Polynomial.h>

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

  static T boundary(int index) {
    return -0.5*support() + T(index);
  }

  SplineBasis<T, PieceCount+1> next() {
    auto left = Polynomial<T, 2>{-0.5, 1.0};
    auto right = Polynomial<T, 2>{0.5, 1.0};

    SplineBasis<T, PieceCount+1> dst;
    auto leftMost = pieces[0].primitive();
    for (int i = 0; i < PieceCount+1; i++) {
      auto split = Polynomial<T, 1>(boundary(i));
      auto leftPolyPrimitive = get(i).primitive();
      auto rightPolyPrimitive = get(i+1).primitive();
      dst.pieces[i] =
          eval(leftPolyPrimitive, split)
            - eval(leftPolyPrimitive, left)
          + eval(rightPolyPrimitive, right)
           - eval(rightPolyPrimitive, split);
    }
    return dst;
  }
};

} /* namespace sail */

#endif /* SERVER_MATH_SPLINE_H_ */
