/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <ceres/ceres.h>
#include <server/math/ADFunction.h>
#include <server/common/string.h>

using namespace sail;

namespace {
  template <typename T>
  T branchFun1(T x) {
    T a = x;
    T b = 0;
    if (x <= 0) {
      b = x*x;
    } else {
      b = exp(x);
    }
    return a + b;
  }

  template <typename T>
  T branchFun2(T x) {
    T a = x;
    return a + (x < 0? x*x : exp(x));
  }
}

TEST(ADBranchTest, Adolc) {
  class TestFun : public AutoDiffFunction {
   public:
    TestFun(bool first) : _first(first) {}
    int inDims() {return 2;}
    int outDims() {return 1;}
    void evalAD(adouble *Xin, adouble *Fout) {
      if (_first) {
        Fout[0] = branchFun1(Xin[0]);
      } else {
        Fout[0] = branchFun2(Xin[0]);
      }
    }
   private:
    bool _first;
  };

  double x[2] = {0.0, 0.0};
  {
    TestFun f(true);
    double F[1], J[2];
    f.eval(x, F, J);
    EXPECT_TRUE(std::isnan(J[0]));
    std::cout << EXPR_AND_VAL_AS_STRING(J[0]) << std::endl;
  }

}



