/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef QUADFUN_H_
#define QUADFUN_H_

#include <server/common/SymmetricMatrix.h>

namespace sail {

template <int xDims, int rhsDims, typename T = double>
class QuadFun {
 public:
  static constexpr int pDims = xDims*xDims;
  static constexpr int qDims = xDims*rhsDims;
  static constexpr int rDims = rhsDims*rhsDims;
 private:
  T _P[pDims];
  T _Q[xDims*rhsDims];
  T _R[rhsDims*rhsDims];
};


}




#endif /* QUADFUN_H_ */
