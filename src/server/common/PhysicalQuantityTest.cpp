/*
 *  Created on: 2014-02-13
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <cmath>
#include "PhysicalQuantity.h"
#include <gtest/gtest.h>

using namespace sail;

TEST(PhysQuantTest, CircumferenceTest) {
  Length<double> circumference = Length<double>::fromMetres(4.0e7);
  double minutes = 60.0*Angle<double>::fromRadians(2.0*M_PI).toDegrees();
  double oneNauticalMileMetres = circumference.toMetres()/minutes;
  EXPECT_NEAR(oneNauticalMileMetres, Length<double>::fromNauticalMiles(1.0).toMetres(), 30.0);
  EXPECT_NEAR(circumference.toNauticalMiles(), minutes, 40);
}

TEST(PhysQuantTest, TimeTest) {
  double n = 34.0;
  EXPECT_NEAR(Time<double>::fromSeconds(n).toSeconds(), n, 1.0e-9);
  EXPECT_NEAR(Time<double>::fromSeconds(60).toMinutes(), 1.0, 1.0e-9);
}

TEST(PhysQuantTest, HydroptereTest) {
  // « sustaining a speed of 52.86 knots (97.90 km/h; 60.83 mph) »
  Velocity<double> va = Velocity<double>::fromKnots(52.86);
  Velocity<double> vb = Velocity<double>::fromKilometresPerHour(97.90);
  Velocity<double> vc = Velocity<double>::fromMilesPerHour(60.83);
  EXPECT_NEAR(va.toMetresPerSecond(), vb.toMetresPerSecond(), 0.1);
  EXPECT_NEAR(va.toMetresPerSecond(), vc.toMetresPerSecond(), 0.1);
}

TEST(PhysQuantTest, WalkTest) {
  // When I am in a hurry, I walk
  Length<double> length = Length<double>::fromKilometres(1.0);
  // in
  Time<double> time = Time<double>::fromMinutes(10);

  // Then my speed is
  Velocity<double> velA = Velocity<double>::fromKilometresPerHour(length.toKilometres()/time.toHours());
  Velocity<double> velB = Velocity<double>::fromMetresPerSecond(length.toMetres()/time.toSeconds());

  EXPECT_NEAR(velA.toKnots(), velB.toKnots(), 1.0e-5);
}

TEST(PhysQuantTest, AngleTest) {
  Angle<double> a = Angle<double>::fromDegrees(30);
  Angle<double> b = Angle<double>::fromDegrees(60);
  Angle<double> a2 = Angle<double>::fromRadians(M_PI/6.0);
  Angle<double> b2 = Angle<double>::fromRadians(M_PI/3.0);
  EXPECT_NEAR(a.toDegrees(), a2.toDegrees(), 1.0e-5);
  EXPECT_NEAR(b.toDegrees(), b2.toDegrees(), 1.0e-5);
  EXPECT_NEAR(sin(a), 1.0/2.0, 1.0e-6);
  EXPECT_NEAR(sin(b), sqrt(3)/2.0, 1.0e-6);
  EXPECT_NEAR(sin(a)*sin(a) + cos(a)*cos(a), 1.0, 1.0e-6);
  EXPECT_NEAR(cos(a2)*cos(b2) - sin(a2)*sin(b2), cos(a.toRadians() + b.toRadians()), 1.0e-5);
}


/*
 * Test of how the Curiously Recurring Template pattern
 * can make the code somewhat cleaner.
 */
namespace {
template <typename Quantity, typename ValueType>
class PQ {
 protected:
  void set(ValueType x) {_x = x;}
  ValueType _x;
};

#define MAKE_PQ_CONSTRUCTORS(ClassName) \
  private: Mass(T x) {this->set(x);} \
  public: Mass() {this->set(0);} \
    typedef ClassName<T> ThisType;

#define MAKE_PQ_CONVERTERS(fromName, toName, fromFactor) \
  static ThisType fromName(T x) {return ThisType((fromFactor)*x);} \
  T toName() const {return (1.0/(fromFactor))*(this->_x);}

template <typename T>
class Mass : public PQ<Mass<T>, T> {
  MAKE_PQ_CONSTRUCTORS(Mass)
  MAKE_PQ_CONVERTERS(kilograms, toKilograms, 1.0);
  MAKE_PQ_CONVERTERS(skeppund, toSkeppund, 170.0);
  MAKE_PQ_CONVERTERS(lispund, toLispund, 170.0/20.0);
};

#undef MAKE_PQ_CONSTRUCTORS
}

TEST(PhysQuantTest, NewPQTest) {

}
