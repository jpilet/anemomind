/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "CleanNumArray.h"
#include <server/math/BandMat.h>
#include <server/common/math.h>

namespace sail {

Arrayd cleanNumArray(Arrayd arr) {
  int count = arr.size();
  BandMatd A(count, count, 2, 2);
  MDArray2d B(count, 1);
  B.setAll(0);
  Arrayi orders = Arrayi::args(2);
  Arrayd weights = Arrayd::args(1.0e-6);
  A.addRegs(orders, weights);
  for (int i = 0; i < count; i++) {
    double x = arr[i];
    if (isOrdinary(x)) {
      int I[1] = {i};
      double W[1] = {1.0};;
      A.addNormalEq(1, I, W);
      B(i, 0) = x;
    }
  }
  bandMatGaussElimDestructive(&A, &B);
  return B.getStorage();
}

}
