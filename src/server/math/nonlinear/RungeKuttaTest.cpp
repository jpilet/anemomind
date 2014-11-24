/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/math/nonlinear/RungeKutta.h>
#include <server/common/SharedPtrUtils.h>
#include <cmath>

using namespace sail;

TEST(RungeKutta, BasicDifEq) {

  // Solve the differential equation dx/dt = 5 - x.
  //
  // Given x(0) = 3.0,
  //
  // The analytic solution to this equation is
  //
  // x(t) = 5 - 2.0*exp(-t)
  //

  double startValue = 3.0;
  double targetValue = 5.0;

  class Eq : public Function {
   public:
    Eq(double tgt) : _tgt(tgt) {}
    int inDims() {return 1;}
    int outDims() {return 1;}
    void eval(double *Xin, double *Fout, double *Jout) {
      assert(Jout == nullptr); // <-- Runge-Kutta doesn't use the Jacobian.
      Fout[0] = _tgt - Xin[0];
    }
   private:
    double _tgt;
  };

  Eq eq(targetValue);
  RungeKutta rk(makeSharedPtrToStack(eq));

  int count = 300;
  double stepSize = 0.05;

  double xEst = startValue;
  double time = 0.0;
  Arrayd stateVector(1, &xEst);

  double totalTime = count*stepSize;
  for (double time = 0; time < totalTime; time++) {
    double xGt = 5.0 - 2.0*exp(-time);
    EXPECT_NEAR(xGt, xEst, 0.01);
    rk.step(&stateVector, stepSize);
  }
}
