/*
 *  Created on: 2014-03-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "LU.h"
#include "LUImpl.h"

namespace sail {


template <typename T>
arma::Mat<T> solveLUT(arma::Mat<T> A, arma::Mat<T> B) {
  assert(A.n_rows == A.n_cols);
  arma::Mat<T> X(A.n_cols, B.n_cols);
  MDArray<T, 2> Aarr = toMDArray(A);
  MDArray<T, 2> Barr = toMDArray(B);
  MDArray<T, 2> Xarr = toMDArray(X);
  T *xinit = Xarr.ptr();
  assert(xinit == X.memptr());
  LUImpl::solveLinearSystemLU<T>(Aarr, Barr, &Xarr);
  assert(xinit == Xarr.ptr());
  assert(Xarr.ptr() == X.memptr());
  return X;
}

arma::mat solveLU(arma::mat A, arma::mat B) {
  return solveLUT<double>(A, B);
}

arma::admat solveLU(arma::admat A, arma::admat B) {
  return solveLUT<adouble>(A, B);
}

void solveLUArrayOut(MDArray2d A, MDArray2d b, MDArray2d *xOut) {
  LUImpl::solveLinearSystemLU<double>(A, b, xOut);
}

void solveLUArrayOut(MDArray2ad A, MDArray2ad b, MDArray2ad *xOut) {
  LUImpl::solveLinearSystemLU<adouble>(A, b, xOut);
}



} /* namespace mmm */
