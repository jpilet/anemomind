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
class DimList {
 public:
  DimList() : _dimIndex(Dim-1) {}
  DimList<Dim-1> remove(int dim) const {
    assert(dim >= 0);
    if (dim == 0) {
      return _next;
    } else {
      return DimList<Dim-1>(_dimIndex, _next.remove(dim-1));
    }
  }
 private:
  DimList(int index_, const DimList<Dim-1> &next_) :
    _dimIndex(index_), _next(next_) {}
  int _dimIndex;
  DimList<Dim-1> _next;
};

template <>
class DimList<0> {
 public:
  DimList() {}
};

template <>
class DimList<1> {
 public:
  DimList() : _dimIndex(0) {}
  DimList(int index_, const DimList<0> &next) : _dimIndex(index_) {}
  DimList<0> remove(int dim) const {
    return DimList<0>();
  }
 private:
  int _dimIndex;
};




template <int Dim>
class IndexBox {
 public:
  typedef IndexBox<Dim-1> NextBox;
  typedef IndexBox<Dim> ThisType;
  IndexBox() : _actualSize(0), _offset(0), _size(0) {}

  IndexBox(int size_) : _actualSize(size_), _offset(0), _size(size_) {}

  static ThisType sameSize(int size) {
    return IndexBox(size, 0, size, NextBox::sameSize(size));
  }

  ThisType slice(int dim, int from, int to) const {
    if (dim == 0) {
      return IndexBox(_actualSize, _offset + from, to - from, _next);
    } else {
      return IndexBox(_actualSize, _offset, _size, _next.slice(dim-1, from, to));
    }
  }

  ThisType slice(int dim, int index) const {
    return slice(dim, index, index + 1);
  }

  int numel() const {
    return _size*_next.numel();
  }


  // Public so that the + operator can use it.
  IndexBox(int actualSize_, int offset_, int size_,
    const NextBox &next_) : _actualSize(actualSize_),
    _offset(offset_), _size(size_), _next(next_) {}

  int actualSize() const {return _actualSize;}
  int offset() const {return _offset;}
  int size() const {return _size;}

  int calcIndex(const int *inds) const {
    assert(0 <= inds[0] && inds[0] < _size);
    return withOffset(inds[0]) + _actualSize*_next.calcIndex(inds + 1);
  }

  bool hasMidpoint() const {
    return (_size % 2 == 1) && _next.hasMidpoint();
  }

  int lowIndex() const {
    return _offset + _actualSize*_next.lowIndex();
  }

  int highIndex() const {
    return withOffset(_size-1) + _actualSize*_next.highIndex();
  }

  int midpointIndex() const {
    return withOffset(_size/2) + _actualSize*_next.midpointIndex();
  }

 private:
  int withOffset(int x) const {return x + _offset;}
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

  ThisType slice(int dim, int from, int to) const {
    return ThisType();
  }

  int numel() const {
    return 1;
  }

  int calcIndex(const int *inds) const {
    return 0;
  }

  bool hasMidpoint() const {return true;}

  int midpointIndex() const {return 0;}
  int lowIndex() const {return 0;}
  int highIndex() const {return 0;}
 private:
};

template <int Dim>
IndexBox<Dim+1> operator+(const IndexBox<1> &a, const IndexBox<Dim> &b) {
  return IndexBox<Dim+1>(a.actualSize(), a.offset(), a.size(), b);
}




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
