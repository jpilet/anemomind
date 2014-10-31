/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "CleanNumArray.h"
#include <server/math/BandMat.h>
#include <server/common/math.h>
#include <server/math/ContinuousAngles.h>
#include <iostream>

namespace sail {

Arrayd cleanNumArray(Arrayd arr) {
  int count = arr.size();
  BandMatd A(count, count, 2, 2);
  MDArray2d B(count, 1);
  B.setAll(0);
  Arrayi orders = Arrayi::args(2);
  Arrayd weights = Arrayd::args(1.0e-2);
  A.addRegs(orders, weights);
  for (int i = 0; i < count; i++) {
    double x = arr[i];
    if (std::isfinite(x)) {
      int I[1] = {i};
      double W[1] = {1.0};;
      A.addNormalEq(1, I, W);
      B(i, 0) = x;
    }
  }
  assert(bandMatGaussElimDestructive(&A, &B, 1.0e-12));
  return B.getStorage();
}

Array<Angle<double> > cleanContinuousAngles(Array<Angle<double> > allAngles) {
  Array<Angle<double> > goodAngles = allAngles.slice([](Angle<double> x) {
    return std::isfinite(x.degrees());
  });
  Array<Angle<double> > contAngles = makeContinuousAngles(goodAngles);
  int counter = 0;
  int count = allAngles.size();
  Array<Angle<double> > clean(count);
  for (int i = 0; i < count; i++) {
    if (std::isfinite(allAngles[i].degrees())) {
      clean[i] = contAngles[counter];
      counter++;
    }
  }
  assert(counter == contAngles.size());
  Arrayd degs = clean.map<double>([](Angle<double> x) {return x.degrees();});
  Arrayd cleaned = cleanNumArray(degs);
  assert(!degs.empty());
  assert(!cleaned.empty());
  return cleaned
      .map<Angle<double> >([=](double x) {return Angle<double>::degrees(x);});
}

}
