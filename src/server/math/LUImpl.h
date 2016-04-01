/*
 *  Created on: 2014-03-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Implementation of LU decomposition designed to work with automatic differentiation.
 *
 *  For ADOL-C support, <adolc/adouble.h> must be included before
 *  this file.
 *
 *  Differentiability of a linear system (X is the unknown):
 *
 *  A*X = B  =>  D(A*X) = D(B)  <=>  D(A)*X + A*D(X) = D(B)  <=>  D(X) = A\(D(B) - D(A)*X)
 */

#ifndef LUIMPL_H_
#define LUIMPL_H_

#include <server/common/MDArray.h>
#include <server/common/ToDouble.h>
#include <algorithm> // <-- to get std::swap

namespace sail { namespace LUImpl {

void initP(int n, Array<int> *Pout);

// Eldén, Numeriska Beräkningar, page 198
template <typename T>
void pivot(MDArray<T, 2> *Cio, Array<int> *Dio, int k) {
  MDArray<T, 2> &C = *Cio;
  Arrayi &D = *Dio;
  int n = C.rows();
  assert(k < n);
  assert(n == C.cols());
  assert(n == D.size());

  T y = 0;
  int ind = k;
  for (int i = k; i < n; i++) {
    T x = fabs(C.get(i, k));
    // IMPORTANT!!! We must wrap with ToDouble here when using auto diff types, such as adouble.
    // Otherwise, it will appear as if the problem is not differentiable when x = y (even if it is in reality)
    if (ToDouble<T>(x) > ToDouble<T>(y)) {
      y = x;
      ind = i;
    }
  }
  if (k != ind) {
    std::swap(D[k], D[ind]);
    for (int j = 0; j < n; j++) {
      std::swap(C(k, j), C(ind, j));
    }
  }
}


template <typename T>
MDArray<T, 2> initAResult(MDArray<T, 2> &A) {
  int n = A.rows();
  assert(n == A.cols());
  MDArray<T, 2> B(n, n);
  B.setAll(0.0);
  return B;
}

template <typename T>
MDArray<T, 2> triU(MDArray<T, 2> A) {
  MDArray<T, 2> B = initAResult(A);
  int n = A.rows();
  for (int i = 0; i < n; i++) {
    for (int j = i; j < n; j++) {
      B(i, j) = A(i, j);
    }
  }
  return B;
}

template <typename T>
MDArray<T, 2> triL(MDArray<T, 2> A) {
  MDArray<T, 2> B = initAResult(A);
  int n = A.rows();
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < i; j++) {
      B(i, j) = A(i, j);
    }
    B(i, i) = 1.0;
  }
  return B;
}

template <typename T>
void decomposeLUDestructive(MDArray<T, 2> *Aio, Array<int> *Pout) {
  MDArray<T, 2> &A = *Aio;
  Arrayi &P = *Pout;
  int n = A.rows();
  assert(n == A.cols());
  initP(n, &P);
  int butLast = n - 1;
  for (int k = 0; k < butLast; k++) {
    pivot<T>(&A, &P, k);
    T Akk = A.get(k, k);

    for (int i = k+1; i < n; i++) {
      A(i, k) /= Akk;
    }

    for (int i = k+1; i < n; i++) {
      for (int j = k+1; j < n; j++) {
        A(i, j) = A(i, j) - A(i, k)*A(k, j);
      }
    }
  }
}


// Eldén, Numeriska Beräkningar, page 211
template <typename T>
void decomposeLU(MDArray<T, 2> Ain, MDArray<T, 2> *Aout, Array<int> *Pout) {
  *Aout = Ain.dup();
  decomposeLUDestructive<T>(Aout, Pout);
}



template <typename T>
MDArray<T, 2> calcPb(Array<int> P, MDArray<T, 2> b) {
  int n = P.size();
  int rhs = b.cols();
  MDArray<T, 2> y(n, rhs);
  for (int i = 0; i < n; i++) {
    for (int k = 0; k < rhs; k++) {
      y(i, k) = b(P[i], k);
    }
  }
  return y;
}

// Eldén, Numeriska Beräkningar, page 212
template <typename T, typename B>
void solveLyPb(MDArray<T, 2> L, Array<int> P, B b, MDArray<T, 2> &y) {
  int n = L.rows();
  assert(n == L.cols());
  assert(n == P.size());
  assert(n == b.rows());
  int rhs = b.cols();
  y.create(n, rhs);
  for (int i = 0; i < n; i++) {
    for (int k = 0; k < rhs; k++) {
      y(i, k) = b(P[i], k);
    }

    for (int j = 0; j < i; j++) {
      T Lij = L(i, j);
      for (int k = 0; k < rhs; k++) {
        y(i, k) = y(i, k) - Lij*y(j, k);
      }
    }
  }
}

// Eldén, Numeriska Beräkningar, page 188
template <typename T>
void backsubst(MDArray<T, 2> R, MDArray<T, 2> C, MDArray<T, 2> &X) {
  int n = R.rows();
  assert(n == R.cols());
  assert(n == C.rows());
  int rhs = C.cols();
  X.create(n, rhs);
  int last = n-1;
  T Rnn = R(last, last);
  for (int j = 0; j < rhs; j++) {
    X(last, j) = C(last, j)/Rnn;
  }

  for (int i = n-2; i >= 0; i--) {
    T Rii = R(i, i);
    for (int k = 0; k < rhs; k++) {
      X(i, k) = C(i, k);
      for (int j = i+1; j < n; j++) {
        X(i, k) = X(i, k) - R(i, j)*X(j, k);
      }
      X(i, k) = X(i, k)/Rii;
    }
  }
}


// Eldén, Numeriska Beräkningar, page 211
template <typename T>
void solveLinearSystemLU(MDArray<T, 2> A, MDArray<T, 2> b, MDArray<T, 2> *xOut) {
  MDArray<T, 2> LR, y;
  Array<int> P;
  decomposeLU<T>(A, &LR, &P);
  solveLyPb<T>(LR, P, b, y);
  backsubst<T>(LR, y, *xOut);
}





}}

#endif /* LUIMPL_H_ */
