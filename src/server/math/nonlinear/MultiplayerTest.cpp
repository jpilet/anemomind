/*
 *  Created on: 2014-04-23
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/math/nonlinear/Multiplayer.h>
#include <server/math/nonlinear/StepMinimizer.h>
#include <server/common/SharedPtrUtils.h>
#include <server/common/string.h>
#include <server/common/ArrayIO.h>
#include <armadillo>

using namespace sail;

namespace {
  class Valley : public Function {
   public:
    Valley(Arrayd N, double D) : _N(N), _D(D) {}
    int inDims() {return _N.size();}
    int outDims() {return 1;}
    void eval(double *Xin, double *Fout, double *Jout);
    Arrayd normal() {return _N;}
    double offset() {return _D;}
   private:
    Arrayd _N;
    double _D;
  };

  void Valley::eval(double *Xin, double *Fout, double *Jout) {
    assert(Jout == nullptr);
    double f = -_D;
    int n = inDims();
    for (int i = 0; i < n; i++) {
      f += _N[i]*Xin[i];
    }
    *Fout = f*f;
  }
}

TEST(MultiplayerTest, Valleys) {
  Valley a(Arrayd{1.0, 34.0}, 0.8);
  Valley b(Arrayd{-39.0, 0.89}, -9);

  arma::mat A(2, 2);
  arma::mat B(2, 1);
  for (int i = 0; i < 2; i++) {
    A(0, i) = a.normal()[i];
    A(1, i) = b.normal()[i];
  }
  B(0, 0) = a.offset();
  B(1, 0) = b.offset();
  arma::mat Xgt = arma::solve(A, B);


  Arrayd Xinit = Arrayd::fill(2, 119.0);

  StepMinimizer minimizer(300);

  Array<std::shared_ptr<Function> > objfs(2);
  objfs[0] = makeSharedPtrToStack<Function>(a);
  objfs[1] = makeSharedPtrToStack<Function>(b);

  Arrayd Xopt = optimizeMultiplayer(minimizer, objfs, Xinit, Arrayd::fill(2, 1.0));
  EXPECT_NEAR(Xopt[0], Xgt[0], 1.0e-3);
  EXPECT_NEAR(Xopt[1], Xgt[1], 1.0e-3);
}


