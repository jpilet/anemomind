/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TRIBASIS_H_
#define TRIBASIS_H_

#include <armadillo>
#include <server/common/math.h>

namespace sail {

template <int dims>
class TriBasis {
  public:
   typedef arma::Mat<double>::fixed<dims, dims> Matrix;
  private:
   Matrix _A, _Ainv;
  public:

  TriBasis() {
    for (int i = 0; i < dims; i++) {
      double *dst = _A.memptr() + 4*dims;
      makeTriBasisVector(dims, i, dst);
    }
    _Ainv = arma::inv(_A);
  }

  template <typename T>
  T toBasis(const T &x) const {
    return T(_Ainv*x);
  }

  template <typename T>
  T fromBasis(const T &x) const {
    return T(_A*x);
  }

  const Matrix &A() const {return _A;}
  const Matrix &Ainv() const {return _Ainv;}

};

}




#endif /* TRIBASIS_H_ */
