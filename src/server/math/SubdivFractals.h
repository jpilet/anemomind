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
class IndexBox {
 public:
  typedef IndexBox<Dim-1> NextBox;
  typedef IndexBox<Dim> ThisType;
  IndexBox() : _actualSize(0), _offset(0), _size(0) {}

  static ThisType sameSize(int size) {
    return IndexBox(size, 0, size, NextBox::sameSize(size));
  }

 private:
  IndexBox(int actualSize, int offset, int size,
    const NextBox &next) : _actualSize(actualSize),
    _offset(offset), _size(size), _next(next) {}

  int _actualSize;
  int _offset, _size;
  NextBox _next;
};

template <>
class IndexBox<0> {
 public:
  typedef IndexBox<0> ThisType;

  IndexBox() {}

  static ThisType sameSize(int size) {
    return ThisType();
  }

  ThisType slice(int dim, int from, int to) {
    return ThisType();
  }
 private:
};


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

  template <typename VertexType, typename CoordType=double>
  VertexType eval(CoordType coords[Dim], VertexType ctrl[ctrlCount()]) const {
    VertexType vertices[vertexCount()];

  }
 private:
  MDArray2i _subdivIndex;
  MDArray2d _subdivLambda;
};


}




#endif /* SUBDIVFRACTALS_H_ */
