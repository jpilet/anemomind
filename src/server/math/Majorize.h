/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_MATH_MAJORIZE_H_
#define SERVER_MATH_MAJORIZE_H_

#include <server/common/LineKM.h>

namespace sail {


// A majorizing quadratic function
//
// on the form a*x^2 + b*x
//
// The constant term is ignored, because it is usually not needed.
struct MajQuad {
 double a, b;

 bool constant() const {
   return a == 0 && b == 0;
 }

 bool linear() const {
   return a == 0;
 }

 // the function (x - observation)^2
 static MajQuad fit(double observation) {
   return MajQuad{1.0, -2*observation};
 }

 static MajQuad linear(double slope) {
   return MajQuad{0.0, slope};
 }

 // Majorize a function at x, where the function evaluates to f and its derivative to
 // fPrime. The function is assumed to have minimum value a minX.
 static MajQuad majorize(double x, double f, double fPrime, double minX = 0.0) {
   double alpha = fPrime/(2.0*(x - minX));
   return MajQuad{alpha, -2.0*alpha*minX};
 }

 // Square completion: squareFactor()*(x - innerConstant())^2
 double squareFactor() const {return a;}
 double innerConstant() const {return -0.5*b/a;}

 // Represent it as factor()^2, that is on the form (k*x - m)^2
 LineKM factor() const {
   if (a == 0) {
     return LineKM(0, 0);
   }
   double w = sqrt(std::abs(a));
   return LineKM(w, -0.5*b/w);
 }

 //double varWeight() const {return sqrt(a);}
 //double dataWeight() const {return -0.5*b/sqrt(a);}


 // Eval, ignoring the constant.
 double eval(double x) const {
   return a*x*x + b*x;
 }
};

inline MajQuad operator*(double s, const MajQuad &x) {
  return MajQuad{s*x.a, s*x.b};
}

inline MajQuad operator+(const MajQuad &x, const MajQuad &y) {
  return MajQuad{x.a + y.a, x.b + y.b};
}

inline MajQuad operator-(const MajQuad &x, const MajQuad &y) {
  return MajQuad{x.a - y.a, x.b - y.b};
}

inline MajQuad operator-(const MajQuad &x) {
  return MajQuad{-x.a, -x.b};
}


}



#endif /* SERVER_MATH_MAJORIZE_H_ */
