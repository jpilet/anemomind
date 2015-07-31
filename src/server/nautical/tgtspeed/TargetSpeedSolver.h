/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *
 *  An optimization algorithm that fits a target speed function
 *  to observations. It minimizes a sum of errors between a fitted
 *  target speed surface and observations, weighted by the stability
 *  of each observation, along with some regularization.
 *
 *  A function is of type DataCost and is applied to each error before they are summed up.
 *  This function can be nonsmooth, for instance BalancedCost.
 *
 *  The optimization is done using the Majorize-Minimize strategy. In each iteration,
 *  a quadratic majorizer is computed. This majorizer is minimized using the NNLS
 *  algorithm, to constrain the boat speed to increase as the wind speed increases.
 *
 */

#ifndef SERVER_NAUTICAL_TGTSPEED_TARGETSPEEDSOLVER_H_
#define SERVER_NAUTICAL_TGTSPEED_TARGETSPEEDSOLVER_H_

#include <server/nautical/tgtspeed/TargetSpeedParam.h>
#include <server/nautical/tgtspeed/TargetSpeedPoint.h>
#include <server/math/GridSearch.h>


namespace sail {
namespace TargetSpeedSolver {


// A data point to fit in the optimization
struct Point {
 TargetSpeedParam::Loc loc;
 Velocity<double> value;
 double stability;
};


// A majorizing quadratic function
//
// on the form a*x^2 + b*x
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

 // Square completion: squareFactor()*(x - innerConstant())^2
 double squareFactor() const {return a;}
 double innerConstant() const {return -0.5*b/a;}

 LineKM factor() const {
   if (a == 0) {
     return LineKM(0, 0);
   }
   double w = sqrt(std::abs(a));
   return LineKM(w, -0.5*b/w);
 }

 double varWeight() const {return sqrt(a);}
 double dataWeight() const {return -0.5*b/sqrt(a);}
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


class DataCost {
 public:
  virtual double eval(Velocity<double> surface, const Point &pt) const = 0;
  virtual MajQuad majorize(Velocity<double> surface, const Point &pt) const = 0;
  virtual ~DataCost() {}
};


// The absolute value plus a slope.
class BalancedCost : public DataCost {
 public:
  static constexpr double defaultBalance = 0.90;

  BalancedCost(double b = defaultBalance) : _balance(b) {}
  virtual double eval(Velocity<double> surface, const Point &pt) const;
  virtual MajQuad majorize(Velocity<double> surface, const Point &pt) const;
 private:
  double _balance;
};



struct Settings {
  Settings();
  DataCost *cost; // the user is responsible for managing the lifetime of this object.
  int iters;
  bool weightedByStability;
  double radialReg, angularReg;
  int radialRegOrder, angularRegOrder;

  const DataCost *getDataCostOrDefault() const;
};

struct Results {
 TargetSpeedFunction targetSpeed;
 double dataCost;
 Settings settings;
 Arrayd rawVerticesKnots;
};


/*
 *
 *
 *
 * The main function to call in order to fit a target speed function
 * to observations.
 *
 *
 */
Results optimize(TargetSpeedParam param,
    Array<TargetSpeedPoint> points, Settings settings = Settings());





// Auto-tuned version.
struct TuneSettings {
 TuneSettings();

 GridSearch::Settings gridSearchSettings;

 // The factor by which a regularization parameter is changed.
 double changeFactor;

 // How to split the data for cross validation
 int setCount, chunkSize;

 // A smaller number of iterations for the inner optimization, to save time.
 int reducedIters;
};


// Using cross validation.
Settings optimizeParameters(TargetSpeedParam param,
    Array<TargetSpeedPoint> points, TuneSettings tuneSettings,
      Settings settings = Settings());


}
}

#endif
