/*
 *  Created on: 2014-03-13
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef BANDMAT_H_
#define BANDMAT_H_

#include <server/common/MDArray.h>

namespace sail {

template <typename T>
class BandMat {
 public:
  BandMat() : _rows(-1), _cols(-1), _left(-1), _right(-1) {}
  BandMat(int rows, int cols, int left, int right,
      MDArray<T, 2> data = MDArray<T, 2>()) :
    _rows(rows), _cols(cols), _left(left), _right(right) {
    initStorage(data);
  }


  T get(int i, int j) const {
    int j0 = calcCol(i, j);
    if (_data.valid(i, j0)) {
      return _data(i, j0);
    } else {
      return 0.0;
    }
  }

  void set(int i, int j, T x) {
    int j0 = calcCol(i, j);
    assert(_data.valid(i, j0));
    _data(i, j) = x;
  }

  T &operator() (int i, int j) {
    int j0 = calcCol(i, j);
    return _data(i, j0);
  }

  int rows() const {return _rows;}
  int cols() const {return _cols;}
  int left() const {return _left;}
  int right() const {return _right;}
  int leftColIndex(int row) {return std::max(0, row - _left);}
  int rightColIndex(int row) {return std::min(_cols, row + _right + 1);}
  int topRowIndex(int col) {return std::max(0, col - _right);}
  int bottomRowIndex(int col) {return std::min(_rows, col + _left + 1);}
  MDArray<T, 2> toDense() {
    MDArray<T, 2> dst(_rows, _cols);
    dst.setAll(0.0);
    for (int i = 0; i < _rows; i++) {
      for (int j = 0; j < _cols; j++) {
        dst(i, j) = get(i, j);
      }
    }
    return dst;
  }
 private:
  int calcCol(int i, int j) const { return i + j - _left;}

  int _rows, _cols, _left, _right;
  MDArray<T, 2> _data;

  void initStorage(MDArray<T, 2> data) {
    int rows = _rows;
    int cols = _left + 1 + _right;
    if (data.empty()) {
      _data = MDArray<T, 2>(rows, cols);
    } else {
      assert(data.isMatrix(rows, cols));
      _data = data;
    }
  }
};

namespace BMGE {
  template <typename T>
  void scaleRow(int row, BandMat<T> *Aio, T factor) {
    int from = (*Aio).leftColIndex(row);
    int to = (*Aio).rightColIndex(row);
    for (int col = from; col < to; col++) {
      (*Aio)(row, col) *= factor;
    }
  }

  template <typename T>
  void scaleRow(int row, MDArray<T, 2> *Bio, T factor) {
    int n = Bio->cols();
    for (int col = 0; col < n; col++) {
      (*Bio)(row, col) *= factor;
    }
  }

  template <typename T>
  void scaleRowsTo1(int index, BandMat<T> *Aio, MDArray<T, 2> *Bio, double tol) {
    T x = (*Aio)(index, index);
    assert(fabs(x) > tol);
    T factor = 1.0/x;
    scaleRow(index, Aio, factor);
    scaleRow(index, Bio, factor);
  }

  template <typename T>
  void eliminateSub(int i0, int i1, BandMat<T> *Aio, MDArray<T, 2> *Bio) {
    BandMat<T> &A = *Aio;
    MDArray<T, 2> &B = *Bio;

    T factor = -A.get(i1, i0)/A.get(i0, i0);

    {
      int from = A.leftColIndex(i1);
      int to = A.rightColIndex(i1);
      for (int j = from; j < to; j++) {
        A(i1, j) += factor*A.get(i0, j);
      }
    }{
      int from = 0;
      int to = B.cols();
      for (int j = from; j < to; j++) {
        B(i1, j) += factor*B(i0, j);
      }
    }


  }

  template <typename T>
  void eliminateForward(BandMat<T> *Aio, MDArray<T, 2> *Bio, double tol) {
    BandMat<T> &A = *Aio;
    MDArray<T, 2> &B = *Bio;
    for (int i = 0; i < A.rows(); i++) {
      scaleRowsTo1(i, Aio, Bio, tol);
      int to = A.bottomRowIndex(i);
      for (int idst = i+1; idst < to; idst++) {
        eliminateSub(i, idst, Aio, Bio);
      }
    }
  }

  template <typename T>
  void eliminateBackward(BandMat<T> *Aio, MDArray<T, 2> *Bio) {
    BandMat<T> &A = *Aio;
    MDArray<T, 2> &B = *Bio;
    int rows = A.rows();
    for (int i = rows-1; i >= 0; i--) {
      int from = A.topRowIndex(i);
      for (int idst = from; idst > i; idst++) {
        eliminateSub(i, idst, Aio, Bio);
      }
    }
  }
}

/*
 * Solve a system with a band matrix using Gaussian elimination.
 * No pivoting is used, which can lead to loss of accuracy for small elements.
 * This is checked for with the tol parameter.
 */
template <typename T>
void bandMatGaussElim(BandMat<T> *Aio, MDArray<T, 2> *Bio, double tol = 1.0e-6) {
  using namespace BMGE;
  BandMat<T> &A = *Aio;
  MDArray<T, 2> &B = *Bio;
  eliminateForward(Aio, Bio, tol);
  eliminateBackward(Aio, Bio);
}

}

#endif /* BANDMAT_H_ */
