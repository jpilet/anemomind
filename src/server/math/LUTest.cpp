/*
 *  Created on: 2014-03-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/string.h>
#include "LU.h"
#include <server/math/ADFunction.h>

using namespace sail;

TEST(LUTest, SimpleLUTest) {
  double adata[3*3] = { 0.9649  ,  0.1576  ,  0.9706   , 0.9572   , 0.4854  ,  0.8003  ,  0.1419  ,  0.4218   , 0.9157};
  arma::mat A(adata, 3, 3, false, true);
  arma::mat X(3, 1);

  for (int i = 0; i < 3; i++) {
    X(i, 0) = i;
  }

  arma::mat B = A*X;

  arma::mat X2 = solveLU(A, B);
  EXPECT_NEAR(arma::norm(A*X2 - B, 2), 0.0, 1.0e-6);
}

TEST(LUTest, ManyColRightHandSide) {
  double adata[3*3] = { 0.9649  ,  0.1576  ,  0.9706   , 0.9572   , 0.4854  ,  0.8003  ,  0.1419  ,  0.4218   , 0.9157};
  arma::mat A(adata, 3, 3, false, true);
  arma::mat X(3, 2);

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 2; j++) {
      X(i, j) = i + j;
    }
  }

  arma::mat B = A*X;

  arma::mat X2 = solveLU(A, B);
  EXPECT_NEAR(arma::norm(A*X2 - B, 2), 0.0, 1.0e-6);
  for (int i = 0; i < X.n_rows; i++) {
    for (int j = 0; j < X.n_cols; j++) {
      EXPECT_NEAR(X(i, j), X2(i, j), 1.0e-6);
    }
  }
}

class LUFun : public AutoDiffFunction {
 public:
  LUFun(int unknowns, int rhsCols) : _unknowns(unknowns), _rhsCols(rhsCols) {}
  void evalAD(adouble *Xin, adouble *Fout);
  int inDims();
  int outDims();
 private:
  int AElemCount();
  int BElemCount();
  int XElemCount();
  int _unknowns, _rhsCols;
};

void LUFun::evalAD(adouble *Xin, adouble *Fout) {
  MDArray2ad A(_unknowns, _unknowns, Xin + 0);
  MDArray2ad B(_unknowns, _rhsCols, Xin + AElemCount());
  MDArray2ad F(_unknowns, _rhsCols, Fout);
  solveLUArrayOut(A, B, &F);
  assert(F.ptr() == Fout);
}

int LUFun::inDims() {
  return AElemCount() + BElemCount();
}

int LUFun::outDims() {
  return XElemCount();
}

int LUFun::AElemCount() {
  return _unknowns*_unknowns;
}

int LUFun::BElemCount() {
  return _unknowns*_rhsCols;
}

int LUFun::XElemCount() {
  return _unknowns*_rhsCols;
}


TEST(LUTest, AutoDiffLUTest) {
  // The two first coefficients of the first column of A are selected
  // to be equal in order to expose the problem of apparent non-differentiability.
  double ABdata[3*3 + 3] = {1, 1, 0,  -1, 1, 2,  0, 3, 4, // Elements of 3x3 A matrix, col major order

                            0, 0, 0};                     // Elements of 3x1 B matrix, to be computed

  double *Bdata = ABdata + 9;
  arma::mat A(ABdata, 3, 3, false, true);
  arma::mat B(Bdata, 3, 1, false, true);
  arma::mat Xgt(3, 1);
  Xgt(0, 0) = 0.34;
  Xgt(1, 0) = -1.4;
  Xgt(2, 0) = 5;
  B = A*Xgt;
  EXPECT_EQ(B.memptr(), Bdata);
  EXPECT_NE(Bdata[0], 0);

  LUFun testfun(3, 1);

  double F[3], J[3*12], JNum[3*12];
  testfun.eval(ABdata, F, J);
  testfun.evalNumericJacobian(ABdata, JNum);
  for (int i = 0; i < 3*12; i++) {
    EXPECT_NEAR(J[i], JNum[i], 1.0e-6);
  }
  for (int i = 0; i < 3; i++) {
    EXPECT_NEAR(F[i], Xgt(i, 0), 1.0e-6);
  }

}



