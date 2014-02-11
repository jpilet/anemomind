/*
 * mathutils.cpp
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#include "mathutils.h"

namespace sail {

arma::sp_mat makeSpSel(Arrayb sel) {
  int elemCount = countTrue(sel);
  int count = sel.size();
  arma::umat IJ(2, elemCount);
  arma::vec X(elemCount);

  int counter = 0;
  for (int i = 0; i < count; i++) {
    if (sel[i]) {
      IJ(0, counter) = counter;
      IJ(1, counter) = i;
      X[counter] = 1.0;
      counter++;
    }
  }
  assert(counter == elemCount);
  return arma::sp_mat(IJ, X, elemCount, count);
}

} /* namespace sail */
