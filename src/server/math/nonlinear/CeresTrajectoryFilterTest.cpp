/*
 * CeresTrajectoryFilter.cpp
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#include <server/math/nonlinear/CeresTrajectoryFilter.h>
#include <gtest/gtest.h>
#include <server/common/ArrayBuilder.h>

// Used to represent observations in some optimization context
template <int N, typename T = double>
struct TimedObservation {
  typedef Eigen::Matrix<T, N, 1> Vec;

  TimedObservation() : order(-1), time(NAN) {}
  TimedObservation(int o, T t, const Vec &v) : order(o), time(t), value(v) {}

  int order; // For instance 0 can mean position and 1 can mean velocity
  T time;
  Vec value;

  bool operator<(const TimedObservation<N, T> &other) const {
    return time < other.time;
  }
};


using namespace sail;

using namespace CeresTrajectoryFilter;

namespace {
  auto timeOffset = TimeStamp::UTC(2016, 5, 16, 14, 26, 0);

  TimeStamp toTime(double x) {
    return timeOffset + Duration<double>::seconds(x);
  }

  double fromTime(TimeStamp t) {
    return (t - timeOffset).seconds();
  }

  template <int N>
  typename Types<N>::TimedPosition makeTimedPosition(const TimedObservation<N> &obs) {
    typename Types<N>::Position dst;
    for (int i = 0; i < N; i++) {
      dst[i] = Length<double>::meters(obs.value(i));
    }
    return typename Types<N>::TimedPosition(toTime(obs.time), dst);
  }


  template <int N>
  typename Types<N>::TimedMotion makeTimedMotion(const TimedObservation<N> &obs) {
    typename Types<N>::Motion dst;
    for (int i = 0; i < N; i++) {
      dst[i] = Velocity<double>::metersPerSecond(obs.value(i));
    }
    return typename Types<N>::TimedMotion(toTime(obs.time), dst);
  }


  typedef TimedObservation<1> Obs1;
  Obs1 obs(int order, double time, double x) {
    Eigen::Matrix<double, 1, 1> v;
    v(0) = x;
    return Obs1{order, time, v};
  }

  // Wrapper so that we can run the old unit tests with the new code.
  template <int N>
  Array<Eigen::Matrix<double, N, 1> > filterOld(Arrayd samples,
      const Array<TimedObservation<N> > &observations,
      const CeresTrajectoryFilter::Settings &settings) {
    typedef Types<N> TypesN;

    int obsCount = observations.size();
    ArrayBuilder<typename TypesN::TimedPosition> positions(obsCount);
    ArrayBuilder<typename TypesN::TimedMotion> motions(obsCount);
    for (auto obs: observations) {
      if (obs.order == 0) {
        positions.add(makeTimedPosition(obs));
      } else {
        motions.add(makeTimedMotion(obs));
      }
    }

    ArrayBuilder<TimeStamp> timeSamples(samples.size());
    for (auto t: samples) {
      timeSamples.add(timeOffset + Duration<double>::seconds(t));
    }

    auto results = CeresTrajectoryFilter::filter<N>(
        wrapIndexable<TypeMode::ConstRef>(timeSamples.get()),
        wrapIndexable<TypeMode::ConstRef>(positions.get()),
        wrapIndexable<TypeMode::ConstRef>(motions.get()), settings,
        EmptyArray<typename TypesN::Position>());

    Array<Eigen::Matrix<double, N, 1> > dst(results.size());
    for (int i = 0; i < results.size(); i++) {
      const auto &x = results[i];
      auto &y = dst[i];
      for (int j = 0; j < N; j++) {
        y(j) = x.value[j].meters();
      }
    }
    return dst;
  }
}

TEST(FilterTest, TestNoOutliersPositions) {


  Array<Obs1> observations{
    obs(0, 0.0, 3.4),
    obs(0, 3.0, 5.5)
  };

  Settings settings;
  settings.regWeight = 0.0;
  settings.ceresOptions.minimizer_progress_to_stdout = false;
  auto Y = filterOld(Arrayd{0.0, 3.0}, observations, settings);

  EXPECT_EQ(Y.size(), 2);

  EXPECT_NEAR(Y[0](0), 3.4, 1.0e-6);
  EXPECT_NEAR(Y[1](0), 5.5, 1.0e-6);
}

TEST(FilterTest, TestBoundaryFit) {


  Array<Obs1> observations{
    obs(0, 1.0, 3.4),
    obs(1, 3.0, 1.1) // <-- Should have position 3.4 + (3 - 1)*1.1 = 3.4 + 2.2 = 5.6
  };

  Settings settings;
  settings.regWeight = 0.0;
  settings.ceresOptions.minimizer_progress_to_stdout = false;
  auto Y = filterOld(Arrayd{1.0, 3.0}, observations, settings);

  EXPECT_EQ(Y.size(), 2);

  EXPECT_NEAR(Y[0](0), 3.4, 1.0e-6);
  EXPECT_NEAR(Y[1](0), 5.6, 1.0e-6);
}

TEST(FilterTest, TestBoundaryFit2) {


  Array<Obs1> observations{
    obs(1, 1.0, 1.1),
    obs(0, 3.0, 5.6)
  };

  Settings settings;
  settings.regWeight = 0.0;
  settings.ceresOptions.minimizer_progress_to_stdout = false;
  auto Y = filterOld(
      Arrayd{1.0, 3.0}, observations, settings);

  EXPECT_EQ(Y.size(), 2);

  EXPECT_NEAR(Y[0](0), 3.4, 1.0e-6);
  EXPECT_NEAR(Y[1](0), 5.6, 1.0e-6);
}


TEST(FilterTest, TestInnerDerivativeFit) {

  Array<Obs1> observations{
    obs(1, 2.0, 1.1),
    obs(1, 4.0, 1.1),
    obs(0, 5.0, 5.6)
  };

  Settings settings;
  settings.regWeight = 0.0;
  settings.ceresOptions.minimizer_progress_to_stdout = false;
  auto Y = filterOld(Arrayd{1.0, 3.0, 5.0},
      observations, settings);

  EXPECT_EQ(Y.size(), 3);

  EXPECT_NEAR(Y[0](0), 1.2, 1.0e-6);
  EXPECT_NEAR(Y[1](0), 3.4, 1.0e-6);
  EXPECT_NEAR(Y[2](0), 5.6, 1.0e-6);
}

TEST(FilterTest, EffectOfRegularization) {

  Array<Obs1> observations{
    obs(0, 1.0, 1), // 3.4 - 1.1*2 = 3.4 - 2.2 = 1.2
    obs(0, 2.0, 9), // 5.6 - 1.1*2 = 5.6 - 2.2 = 3.4
    obs(0, 4.0, 2)
  };

  {
    Settings settings;
    settings.regWeight = 0.001;
    settings.ceresOptions.minimizer_progress_to_stdout = false;
    auto Y = filterOld(Arrayd{1.0, 2.0, 4.0},
        observations, settings);
    EXPECT_NEAR(Y[0](0), 1.0, 0.01);
    EXPECT_LT(1.0, Y[0](0));

    EXPECT_NEAR(Y[1](0), 9.0, 0.01);
    EXPECT_LT(Y[1](0), 9.0);

    EXPECT_NEAR(Y[2](0), 2.0, 0.01);
    EXPECT_LT(2.0, Y[2](0));
  }{
    Settings settings;
    settings.regWeight = 1.0e2;
    settings.ceresOptions.minimizer_progress_to_stdout = false;
    auto Y = filterOld(Arrayd{1.0, 2.0, 4.0},
        observations, settings);

    EXPECT_NEAR(Y[0](0), 4.2857, 0.001);
    EXPECT_NEAR(Y[1](0), 4.0714, 0.001);
    EXPECT_NEAR(Y[2](0), 3.6429, 0.001);
  }
}


TEST(FilterTest, RegularizationAndNonUniformSampling) {
  Array<Obs1> observations{
    obs(0, 1.0, 1),
    obs(0, 1.000001, 2),
    obs(0, 2.0, 9)
  };

  Settings settings;
  settings.regWeight = 1.0e4;
  auto Y = filterOld(Arrayd{1.0, 1.00001, 2.0},
      observations, settings);

  EXPECT_NEAR(0.5*(1 + 2), Y[0](0), 1.0e-4);
  EXPECT_NEAR(0.5*(1 + 2), Y[1](0), 1.0e-4);
  EXPECT_NEAR(9.0, Y[2](0), 1.0e-4);

}
