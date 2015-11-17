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
 MajQuad() : a(NAN), b(NAN) {}
 MajQuad(double a_, double b_) : a(a_), b(b_) {}

 bool defined() const {
   return std::isfinite(a);
 }

 double a, b;

 MajQuad mirror() const {
   return MajQuad(a, -b);
 }

 bool isFinite() const {
   return std::isfinite(a) && std::isfinite(b);
 }

 bool constant() const {
   return a == 0 && b == 0;
 }

 bool linear() const {
   return a == 0;
 }

 // the function (x - observation)^2
 static MajQuad fit(double observation) {
   return MajQuad(1.0, -2*observation);
 }

 static MajQuad linear(double slope) {
   return MajQuad(0.0, slope);
 }

 static MajQuad square() {
   return MajQuad(1.0, 0.0);
 }

 double optimimum() const {
   // 2*a*x + b = 0 <=> x = -b/(2*a)
   return -b/(2.0*a);
 }

 // Majorize a function at x, where the function evaluates to f and its derivative to
 // fPrime. The function is assumed to have minimum value a minX.
 static MajQuad majorize(double x, double f, double fPrime, double minX = 0.0) {
   double alpha = fPrime/(2.0*(x - minX));
   return MajQuad(alpha, -2.0*alpha*minX);
 }

 static MajQuad majorizeAbs(double x, double lb) {
   double xp = thresholdCloseTo0(x, lb);
   double f = std::abs(xp);
   double fPrime = (xp < 0? -1 : 1);
   return majorize(xp, f, fPrime, 0.0);
 }

 // Represent it as factor()^2, that is on the form (k*x + m)^2
 LineKM factor() const {
   if (a == 0) {
     return LineKM(0, 0);
   }
   double w = sqrt(std::abs(a));
   return LineKM(w, 0.5*b/w);
 }

 // Eval, ignoring the constant.
 double eval(double x) const {
   return a*x*x + b*x;
 }

 double evalDerivative(double x) const {
   return 2.0*a*x + b;
 }
};

inline MajQuad operator*(double s, const MajQuad &x) {
  return MajQuad(s*x.a, s*x.b);
}

inline MajQuad operator+(const MajQuad &x, const MajQuad &y) {
  return MajQuad(x.a + y.a, x.b + y.b);
}

inline MajQuad operator-(const MajQuad &x, const MajQuad &y) {
  return MajQuad(x.a - y.a, x.b - y.b);
}

inline MajQuad operator-(const MajQuad &x) {
  return MajQuad(-x.a, -x.b);
}


}



#endif /* SERVER_MATH_MAJORIZE_H_ */
