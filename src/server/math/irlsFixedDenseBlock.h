/*
 * irlsFixedDenseBlock.h
 *
 *  Created on: Apr 22, 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_IRLSFIXEDDENSEBLOCK_H_
#define SERVER_MATH_IRLSFIXEDDENSEBLOCK_H_

#include <assert.h>
#include <server/math/irls.h>

namespace sail {
namespace irls {

template <int rows, int lhs, int rhs>
class FixedDenseBlock : public DenseBlock {
public:
  typedef Eigen::Matrix<double, rows, lhs> AType;
  typedef Eigen::Matrix<double, rows, rhs> BType;

  int lhsCols() const override {return lhs;}
  int rhsCols() const override {return rhs;}

  FixedDenseBlock(const AType &A, const BType &B, int row, int col) :
    _A(A), _B(B), _row(row), _col(col) {}

  virtual void accumulateWeighted(const Eigen::VectorXd &weights,
      BandMatrix<double> *AtA, MDArray2d *AtB) const override {
    Eigen::DiagonalMatrix<double, rows> W(weights.block(_row, 0, rows, 1));
    AType WA = W*_A;
    bandMatrixView(AtA, lhs, _col) += WA.transpose()*WA;
    arrayView(AtB, lhs, rhs, _col) += WA.transpose()*W*_B;
  }

  void eval(const Eigen::MatrixXd &X,
      Eigen::VectorXd *residuals) const override {
    assert(X.cols() == rhs);
    auto xsub = X.block(_col, 0, lhs, rhs);
    if (rhs == 1) {
      auto rsub = residuals->block(_row, 0, rows, 1);
      rsub = _A*xsub - _B;
    } else {
      BType tmp = _A*xsub - _B;
      for (int i = 0; i < rows; i++) {
        (*residuals)(_row + i, 0) = tmp.block(i, 0, 1, rhs).norm();
          //tmp.block<1, rhs>(i, 0).norm(); doesn't seem to compile
      }
    }
  }

  int requiredRows() const override {return _row + rows;}
  int requiredCols() const override {return _col + lhs;}
private:
  int _row, _col;
  AType _A;
  BType _B;
};

}
}

#endif /* SERVER_MATH_IRLSFIXEDDENSEBLOCK_H_ */
