/*
 *  Created on: 14 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "ConcatFunction.h"
#include <gtest/gtest.h>

using namespace sail;

namespace {
  class FunA : public Function {
   public:
    int inDims() {return 2;}
    int outDims() {return 1;}
    void eval(double *Xin, double *Fout, double *Jout);
  };

  void FunA::eval(double *Xin, double *Fout, double *Jout) {
    Fout[0] = Xin[0] - Xin[1];
    if (Jout != nullptr) {
      Jout[0] = 1.0;
      Jout[1] = -1.0;
    }
  }

  class FunB : public Function {
   public:
    int inDims() {return 2;}
    int outDims() {return 2;}
    void eval(double *Xin, double *Fout, double *Jout);
  };

  void FunB::eval(double *Xin, double *Fout, double *Jout) {
    Fout[0] = Xin[0] + 3*Xin[1];
    Fout[1] = 3*Xin[0] + 2*Xin[1];
    if (Jout != nullptr) {
      Jout[0] = 1.0;
      Jout[1] = 3.0;
      Jout[2] = 3.0;
      Jout[3] = 2.0;
    }
  }
}

TEST(ConcatFunctionTest, Test1) {
  FunA A;
  FunB B;
  ConcatFunction C(A, B);
  EXPECT_EQ(C.inDims(), 2);
  EXPECT_EQ(C.outDims(), 3);

  double X[2] = {1.1, 2.4};
  double Fgt[3] = {-1.3000 ,   8.3000  ,  8.1000};
  double Jgt[6] = {1,     1,     3,    -1,     3,     2};

  double F0[3];
  C.eval(X, F0, nullptr);

  for (int i = 0; i < 3; i++) {
    EXPECT_NEAR(F0[i], Fgt[i], 1.0e-6);
  }

  double F1[3], J1[6];
  C.eval(X, F1, J1);
  for (int i = 0; i < 3; i++) {
    EXPECT_NEAR(F1[i], Fgt[i], 1.0e-6);
  }

  for (int i = 0; i < 6; i++) {
    EXPECT_NEAR(J1[i], Jgt[i], 1.0e-6);
  }
}



