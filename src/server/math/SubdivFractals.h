/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */
#ifndef SUBDIVFRACTALS_H_
#define SUBDIVFRACTALS_H_

#include <server/common/MDArray.h>
#include <cmath>
#include <iostream>
#include <server/common/string.h>

namespace sail {

template <int Dim>
class IndexList {
 public:
  IndexList() : _size(Dim) {
    for (int i = 0; i < Dim; i++) {
      _inds[i] = i;
    }
  }

  int size() const {
    return _size;
  }

  IndexList remove(int index) const {
    return IndexList(index, *this);
  }

  int operator[] (int index) const {
    return _inds[index];
  }

  bool empty() const {
    return _size == 0;
  }
 private:
  IndexList(int indexToRemove, const IndexList &src) {
    _size = src._size - 1;
    for (int i = 0; i < indexToRemove; i++) {
      _inds[i] = src._inds[i];
    }
    for (int i = indexToRemove+1; i < src._size; i++) {
      _inds[i-1] = src._inds[i];
    }
  }

  int _inds[Dim];
  int _size;
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

  ThisType sliceLow(int dim) const {
    if (dim == 0) {
      return IndexBox(_actualSize, _offset, 1, _next);
    } else {
      return IndexBox(_actualSize, _offset, _size, _next.sliceLow(dim-1));
    }
  }

  ThisType sliceHigh(int dim) const {
    if (dim == 0) {
      return IndexBox(_actualSize, _offset + _size-1, 1, _next);
    } else {
      return IndexBox(_actualSize, _offset, _size, _next.sliceHigh(dim-1));
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

  void calcInds(int index, int *inds) const {
    inds[0] = index % _actualSize;
    _next.calcIndex(index/_actualSize, inds+1);
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

  template <typename VertexType>
  void generate(int *vertexTypes,
                VertexType *vertices,
                const MDArray2i &indexTable,
                const MDArray2d &lambdaTable,
      const IndexList<Dim> &indexList = IndexList<Dim>()) const {
      if (!indexList.empty()) {
        assert(hasMidpoint());
        int low = lowIndex();
        int lowType = vertexTypes[low];
        assert(lowType != -1);

        int middle = midpointIndex();

        int high = highIndex();
        int highType = vertexTypes[high];
        assert(highType != -1);

        double lambda = lambdaTable(lowType, highType);
        vertexTypes[middle] = indexTable(lowType, highType);
        vertices[middle] = (1.0 - lambda)*vertices[low] + lambda*vertices[high];

        for (int ka = 0; ka < indexList.size(); ka++) {
          IndexList<Dim> slicedList = indexList.remove(ka);
          assert(slicedList.size() < indexList.size());
          int kabel = indexList[ka];
          sliceLow(kabel).generate(vertexTypes, vertices,
              indexTable, lambdaTable, slicedList);
          sliceHigh(kabel).generate(vertexTypes, vertices,
              indexTable, lambdaTable, slicedList);
        }
      }
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

  ThisType sliceLow(int index) const {
    return ThisType();
  }

  ThisType sliceHigh(int index) const {
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

  void calcInds(int index, int *inds) const {}
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
  VertexType eval(CoordType coords[Dim], VertexType ctrl[ctrlCount()], int depth) const {
    VertexType vertices[vertexCount()];
    if (depth == 0) {
      IndexBox<Dim> box = IndexBox<Dim>::sameSize(2);
      assert(box.numel() == ctrlCount());
      VertexType result = 0.0*ctrl[0];
      for (int i = 0; i < ctrlCount(); i++) {
        int inds[Dim];
        box.calcInds(i, inds);
        double weight = 1.0;
        for (int j = 0; j < Dim; j++) {
          weight *= (inds[j] == 0? 1.0 - coords[j] : coords[j]);
        }
        result = result + weight*ctrl[i];
      }
      return result;
    } else {
      return eval(coords, ctrl, 0);
    }
  }
 private:
  MDArray2i _subdivIndex;
  MDArray2d _subdivLambda;
};


}




#endif /* SUBDIVFRACTALS_H_ */
