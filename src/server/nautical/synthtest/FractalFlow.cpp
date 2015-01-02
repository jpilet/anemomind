/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/FractalFlow.h>
#include <armadillo>
#include <server/common/math.h>

namespace sail {

namespace {

class TriBasis {
 public:
  TriBasis();
  void mapToBasis(const double src[3], double dst[3]) const;
 private:
  arma::mat33 _B, _Binv;
};

TriBasis::TriBasis() {
  for (int i = 0; i < 3; i++) {
    makeTriBasisVector(3, i, _B.memptr() + 3*i);
  }
  _Binv = arma::inv(_B);
}

void TriBasis::mapToBasis(const double src[3], double dst[3]) const {
  arma::mat dstMat(dst, 3, 1, false, true);
  dstMat = _Binv*arma::vec3(src);
}

double mirror(double x) {
  double period2 = positiveMod(x, 2.0);
  return (period2 < 1? period2 : 2 - period2);
}

void mirrorInPlace3(double xInOut[3]) {
  for (int i = 0; i < 3; i++) {
    xInOut[i] = mirror(xInOut[i]);
  }
}


Flow::VelocityFunction makeVelocityFunction(
                     Length<double> spaceUnit,
                     Duration<double> timeUnit,
                     Velocity<double> outputVelocityUnit,
                     SubdivFractals::FractalFunction<3> flow) {
  static TriBasis basis;
  return [=] (const Flow::ProjectedPosition &p, Duration<double> t) {
    double orthoCoords[3] = {p[0]/spaceUnit, p[1]/spaceUnit, t/timeUnit};
    double triCoords[3] = {NAN, NAN, NAN};
    basis.mapToBasis(orthoCoords, triCoords);
    mirrorInPlace3(triCoords);
    return flow.eval(triCoords)*outputVelocityUnit;
  };
}

}


Flow makeFractalFlow(Length<double> inputSpaceUnit,
                     Duration<double> inputTimeUnit,
                     Velocity<double> outputVelocityUnit,
                     SubdivFractals::FractalFunction<3> xFlow,
                     SubdivFractals::FractalFunction<3> yFlow) {
  return Flow(makeVelocityFunction(inputSpaceUnit, inputTimeUnit, outputVelocityUnit, xFlow),
              makeVelocityFunction(inputSpaceUnit, inputTimeUnit, outputVelocityUnit, yFlow));
}


} /* namespace mmm */
