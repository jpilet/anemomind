/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */
#ifndef SUBDIVFRACTALS_H_
#define SUBDIVFRACTALS_H_

#include <server/common/MDArray.h>
#include <cmath>

namespace sail {

template <int Dim>
class SubdivFractals {
 public:
  static constexpr int vertexDim() {return 3;}
  static constexpr int vertexCount() {return std::pow(vertexDim(), Dim);}
  static constexpr int ctrlDim() {return 2;}
  static constexpr int ctrlCount() {return std::pow(ctrlDim(), Dim);}

  SubdivFractals(MDArray2i subdivIndex, MDArray2d subdivLambda) :
    _subdivIndex(subdivIndex), _subdivLambda(subdivLambda) {
    assert(subdivIndex.isSquare());
    assert(subdivLambda.isSquare());
    int n = subdivIndex.rows();
    assert(n == subdivLambda.rows());
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        assert(std::isfinite(_subdivLambda(i, j)));
        int index = _subdivIndex(i, j);
        assert(0 <= index && index < n);
      }
    }
  }
 private:
  MDArray2i _subdivIndex;
  MDArray2d _subdivLambda;
};


}




#endif /* SUBDIVFRACTALS_H_ */
