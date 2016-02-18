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
#include <server/common/MeanAndVar.h>
#include <device/Arduino/libraries/Corrector/Corrector.h>
#include <device/Arduino/libraries/CalibratedNav/CalibratedNav.h>
#include <server/nautical/FlowErrors.h>
#include <functional>

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

  PhysQuant mean() const {
      PhysQuant dst;
      wrap(_distrib.mean(), &dst);
      return dst;
    }

  PhysQuant stddev() const {
      PhysQuant dst;
      wrap(_distrib.stddev(), &dst);
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
    Corruptor(double scale, T offset, T noiseStd = T::zero()) :
      _distrib(T::zero(), noiseStd), _scale(scale), _offset(offset) {}

    static Corruptor onlyNoise(T noiseStd) {
      return Corruptor(1.0, T::zero(), noiseStd);
    }

    static Corruptor offset(T x) {
      return Corruptor(1.0, x, T::zero());
    }

    static Corruptor scaling(double x) {
      return Corruptor(x, T::zero(), T::zero());
    }

    T corrupt(T value, std::default_random_engine &e) {
      return _scale*value + _offset + _distrib(e);
    }

    void setMapping(double scale, T offset) {
      _scale = scale;
      _offset = offset;
    }

    double scale() const {
      return _scale;
    }

    T offset() const {
      return _offset;
    }

    PhysQuantNormalDistrib<T> distrib() const {
      return _distrib;
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
    Corruptor<Angle<double> > awa, magHdg, gpsBearing;
    Corruptor<Velocity<double> > aws, watSpeed, gpsSpeed;
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
            Array<TwaDirective> dirs_,        // <-- How the boat should be steered
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
    return Duration<double>::seconds(_indexer.sum());
  }

  CorruptedBoatState::CorruptorSet &corruptors() {
    return _corruptors;
  }

  CorruptedBoatState::CorruptorSet corruptors() const {
    return _corruptors;
  }

  Duration<double> samplingPeriod() const {
    return _samplingPeriod;
  }

  int stepsPerSample() const {
    return _stepsPerSample;
  }

  Array<TwaDirective> dirs() const {
    return _dirs;
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

  // The evaluation results for wind or current.
  class SimulatedMotionResults {
   public:
    SimulatedMotionResults() {}
    SimulatedMotionResults(Array<HorizontalMotion<double> > trueMotion,
                Array<HorizontalMotion<double> > estimatedMotion) :
                _trueMotion(trueMotion), _estimatedMotion(estimatedMotion),
                _flowErrors(trueMotion, estimatedMotion) {}
    const FlowErrors &error() const {
      return _flowErrors;
    }
   private:
    FlowErrors _flowErrors;
    Array<HorizontalMotion<double> > _trueMotion, _estimatedMotion;
  };

  class SimulatedCalibrationResults {
   public:
    SimulatedCalibrationResults() {}
    SimulatedCalibrationResults(const SimulatedMotionResults &wind_, const SimulatedMotionResults &current_) :
      _wind(wind_), _current(current_) {}

    const SimulatedMotionResults &wind() const {
      return _wind;
    }

    const SimulatedMotionResults &current() const {
      return _current;
    }
   private:
    SimulatedMotionResults _wind, _current;
  };


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

    NavCollection navs() const {
      return toArray(map(_states, [&](const CorruptedBoatState &s) {
        return s.nav();
      }));
    }

    const BoatSimulationSpecs &specs() const {
      return _specs;
    }

    // Suitable for calibration procedures that don't
    // use the Corruptor class.
    NavalSimulation::SimulatedCalibrationResults evaluateFitness(
        Array<HorizontalMotion<double> > estimatedTrueWindPerNav,
        Array<HorizontalMotion<double> > estimatedTrueCurrentPerNav) const;

    NavalSimulation::SimulatedCalibrationResults evaluateFitness(
        Array<CalibratedNav<double> > cnavs) const;

    NavalSimulation::SimulatedCalibrationResults evaluateNoCalibration() const;

    Array<HorizontalMotion<double> > trueWindOverGround() const;
    Array<HorizontalMotion<double> > trueCurrentOverGround() const;

    void plot() const;

    NavalSimulation::SimulatedCalibrationResults evaluateFitness(const CorrectorFunction &corr) const;
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

  bool hasBoatData() const {
    return _boatData.hasData();
  }

  const BoatData &boatData(int index) const {
    return _boatData[index];
  }

  void setDescription(const std::string &desc) {
    _desc = desc;
  }

  // Constructor used by deserializer
  NavalSimulation(std::string desc,
    GeographicReference geoRef,
    TimeStamp simulationStartTime,
    Array<BoatData> boatData) :
      _desc(desc), _geoRef(geoRef),
      _simulationStartTime(simulationStartTime),
      _boatData(boatData) {}

  TimeStamp simulationStartTime() const {
    return _simulationStartTime;
  }

  std::string description() const {
    return _desc;
  }

  NavalSimulation() {}
 private:
  std::string _desc;
  GeographicReference _geoRef;
  TimeStamp _simulationStartTime;
  Array<BoatData> _boatData;

  FlowFun _wind, _current;

  BoatData makeBoatData(BoatSimulationSpecs &spec,
      Array<BoatSim::FullState> state,
      std::default_random_engine &e) const;
};

std::ostream &operator<< (std::ostream &s, const NavalSimulation::SimulatedCalibrationResults &e);

/*
 * Standard synthetic tests that
 * we will use to evaluate calibration
 * algorithms.
 */

// This tests simulate two boats that sail the
// same trajectory. One boat doesn't have any
// corrupted parameters. The other boat has corrupted
// paramters. Wind and current are constant in space
// and time. The boats try to maintain piecewise constant
// TWA.
NavalSimulation makeNavSimConstantFlow();


// Sail upwind, alternating between TWA of 45 and -45 degs.
// Two different corruptions.
// Wind and current vary.
NavalSimulation makeNavSimUpwindDownwind();
NavalSimulation makeNavSimUpwindDownwindLong();


///////////////////////////////////////
// Simulations using a fractal-based dataset.

/*
 *  - The 'dirs' specify how the boat should sail
 *  - The 'corruptorSets' are different ways of corrupting the measurements
 */
NavalSimulation makeNavSimFractal(
    Array<BoatSimulationSpecs::TwaDirective> dirs,
    Array<CorruptedBoatState::CorruptorSet> corruptorSets);

// An hour of wind oriented race data.
NavalSimulation makeNavSimFractalWindOriented();

// Several hours of wind oriented race data.
NavalSimulation makeNavSimFractalWindOrientedLong();

} /* namespace mmm */

#endif /* TESTCASE_H_ */
