/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TESTCASE_H_
#define TESTCASE_H_

#include <server/nautical/synthtest/BoatSim.h>
#include <server/common/TimeStamp.h>
#include <server/common/ProportionateIndexer.h>
#include <server/nautical/GeographicReference.h>
#include <server/nautical/Nav.h>
#include <random>

namespace sail {


/*
 *  Since the PhysicalQuantity types don't feature
 *  wrap()/unwrap() methods, we have to do some bulky
 *  template specialization and code duplication...
 */
template <typename PhysQuant>
class PhysQuantNormalDistrib {
 public:
  PhysQuantNormalDistrib(PhysQuant mean, PhysQuant stddev)
    : _distrib(unwrap(mean), unwrap(stddev)) {}

  template<typename UniformRandomNumberGenerator>
  PhysQuant operator()(UniformRandomNumberGenerator& urng) {
    PhysQuant dst;
    wrap(_distrib(urng), &dst);
    return dst;
  }
 private:
  std::normal_distribution<double> _distrib;

  template <typename T> static T unwrap(Angle<T> src) {return src.degrees();}
  template <typename T> static void wrap(T x, Angle<T> *dst) {*dst = Angle<T>::degrees(x);}
  template <typename T> static T unwrap(Velocity<T> src) {return src.knots();}
  template <typename T> static void wrap(T x, Velocity<T> *dst) {*dst = Velocity<T>::knots(x);}
};



/*
 * This class holds both the true state of the boat at a time instant
 * and simulated corrupted measurements (corruptedNav) that can be fed
 * to a an algorithm.
 */
class CorruptedBoatState {
 public:
  CorruptedBoatState() {}
  CorruptedBoatState(
      const BoatSim::FullState &trueState,
      const Nav &corruptedNav) : _trueState(trueState),
      _corruptedNav(corruptedNav) {}

  // Return a Nav with corrupted
  const Nav &nav() const {
    return _corruptedNav;
  }

  const BoatSim::FullState &trueState() const {
    return _trueState;
  }


  // This class corrupts a value by scaling error, offset error, and noise.
  template <typename T>
  class Corruptor {
   public:
    Corruptor() : _scale(1.0), _offset(T::zero()), _distrib(T::zero(), T::zero()) {}
    Corruptor(double scale, T offset, T noiseStd) :
      _distrib(T::zero(), noiseStd), _scale(scale), _offset(offset) {}

    static Corruptor onlyNoise(T noiseStd) {
      return Corruptor(1.0, T::zero(), noiseStd);
    }

    T corrupt(T value, std::default_random_engine &e) {
      return _scale*value + _offset + _distrib(e);
    }

    void setMapping(double scale, T offset) {
      _scale = scale;
      _offset = offset;
    }
   private:
    double _scale;
    T _offset;
    PhysQuantNormalDistrib<T> _distrib;
  };

  // This class holds corruptors for all measurements that we simulate.
  // By default there is no corruption.
  class CorruptorSet {
   public:
    typedef Corruptor<Angle<double> > AngleCorr;
    typedef Corruptor<Velocity<double> > VelocityCorr;

    AngleCorr awa, magHdg, gpsBearing;
    VelocityCorr aws, watSpeed, gpsSpeed;
  };
 private:
  BoatSim::FullState _trueState;
  Nav _corruptedNav;
};



// Description of how the testdata should be synthesized
// for a boat:
//  * The dynamics of the boat (BoatCharacteristics)
//  * TWA-directives for how the boat should steer
//  * How the measured values are corrupted
//  * How the boat should be simulated.
class BoatSimulationSpecs {
public:
  BoatSimulationSpecs() : _stepsPerSample(0) {}

  // How the boat should be steered for a portion of time of length 'duration'
  class TwaDirective {
   public:
    TwaDirective() : duration(Duration<double>::seconds(NAN)),
            srcTwa(Angle<double>::degrees(NAN)),
            dstTwa(Angle<double>::degrees(NAN)) {}
    TwaDirective(Duration<double> dur_, Angle<double> srcTwa_, Angle<double> dstTwa_) :
      duration(dur_), srcTwa(srcTwa_), dstTwa(dstTwa_) {}

    static TwaDirective constant(Duration<double> dur, Angle<double> twa) {
      return TwaDirective(dur, twa, twa);
    }

    Duration<double> duration;
    Angle<double> srcTwa, dstTwa;

    // Used by the twa() method of BoatSimDirs
    Angle<double> interpolate(double localTimeSeconds) const {
      double lambda = localTimeSeconds/duration.seconds();
      return (1.0 - lambda)*srcTwa + lambda*dstTwa;
    }
  };

  BoatSimulationSpecs(BoatCharacteristics ch_, // <-- How the boat behaves
            Array<TwaDirective> specs_,        // <-- How the boat should be steered
            CorruptedBoatState::CorruptorSet corruptors_, // <-- How the measurements are corrupted

            Nav::Id boatId = Nav::debuggingBoatId(), // <-- Boat id. Maybe not that interesting in most cases.

            // Settings for how this boat should be simulated.
            Duration<double> samplingPeriod_ = Duration<double>::seconds(1.0),
            int stepsPerSample_ = 20);

  Angle<double> twa(Duration<double> dur) const;

  const Nav::Id &boatId() const {
    return _boatId;
  }

  const BoatCharacteristics &characteristics() const {
    return _ch;
  }

  Duration<double> duration() const {
    Duration<double>::seconds(_indexer.sum());
  }

  CorruptedBoatState::CorruptorSet &corruptors() {
    return _corruptors;
  }

  Duration<double> samplingPeriod() const {
    return _samplingPeriod;
  }

  int stepsPerSample() const {
    return _stepsPerSample;
  }
 private:
  Nav::Id _boatId;
  BoatCharacteristics _ch;
  Array<TwaDirective> _dirs;
  ProportionateIndexer _indexer;
  CorruptedBoatState::CorruptorSet _corruptors;
  Duration<double> _samplingPeriod;
  int _stepsPerSample;
};


/*
 * A Testcase holds testdata for a single race that can
 * contain any number of boats. More specifically, it holds the following:
 *
 * Common to all boats:
 *   * Local wind conditions in space and time
 *   * Local current conditions in space and time
 *   * A geographic reference point
 *   * A time offset
 *
 * Per boat:
 *   * BoatSpecs : Information about how the boat behaves and should steer.
 *   * Array of CorruptedBoatState : An array of simulated boat
 *     states along with corruptions with noise and scaling
 *     to simulate real measurements.
 *
 */
class NavalSimulation {
 public:
  typedef BoatSim::ProjectedPosition ProjectedPosition;
  typedef BoatSim::FlowFun FlowFun;

  static FlowFun constantFlowFun(HorizontalMotion<double> m);


  NavalSimulation(std::default_random_engine &e,
           GeographicReference geoRef,
           TimeStamp simulationStartTime,
           FlowFun wind,
           FlowFun current,
           Array<BoatSimulationSpecs> specs);

  const GeographicReference &geoRef() const {
    return _geoRef;
  }

  Duration<double> toLocalTime(const TimeStamp &x) const {
    return x - _simulationStartTime;
  }

  TimeStamp fromLocalTime(Duration<double> dur) const {
    return _simulationStartTime + dur;
  }

  const FlowFun &wind() const {
    return _wind;
  }

  const FlowFun &current() const {
    return _current;
  }

  // Holds computed data for a boat, ready for testing and evaluating algorithms.
  class BoatData {
   public:
    BoatData() {}
    BoatData(const BoatSimulationSpecs &specs,
        Array<CorruptedBoatState> states) : _specs(specs), _states(states) {}

    const Array<CorruptedBoatState> &states() const {
      return _states;
    }
   private:
    BoatSimulationSpecs _specs;
    Array<CorruptedBoatState> _states;
  };

  int boatCount() const {
    return _boatData.size();
  }

  const Array<BoatData> &boatData() const {
    return _boatData;
  }

  const BoatData &boatData(int index) const {
    return _boatData[index];
  }
 private:
  GeographicReference _geoRef;
  TimeStamp _simulationStartTime;
  FlowFun _wind, _current;

  // Simulated boat states over time along with corrupted measurements.
  Array<BoatData> _boatData;

  BoatData makeBoatData(BoatSimulationSpecs &spec,
      Array<BoatSim::FullState> state,
      std::default_random_engine &e) const;

};

} /* namespace mmm */

#endif /* TESTCASE_H_ */
