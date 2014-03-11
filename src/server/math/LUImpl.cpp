/*
 *  Created on: 10 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "LUImpl.h"

namespace sail { namespace LUImpl {

void initP(int n, Array<int> *Pout) {
  Arrayi &P = *Pout;
  P.create(n);
  for (int i = 0; i < n; i++) {
    P[i] = i;
  }
}

}} /* namespace mmm */
