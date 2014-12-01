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
    : _distrib(mean.unwrap(), stddev.unwrap()) {}

  template<typename UniformRandomNumberGenerator>
  PhysQuant operator()(UniformRandomNumberGenerator& urng) {
    return PhysQuant::wrap(_distrib(urng));
  }
 private:
  std::normal_distribution<double> _distrib;
};

template <>
class PhysQuantNormalDistrib<Angle<double> > {
 public:
  PhysQuantNormalDistrib(Angle<double> mean, Angle<double> stddev) :
    _distrib(mean.degrees(), stddev.degrees()) {}

  template<typename UniformRandomNumberGenerator>
  Angle<double> operator()(UniformRandomNumberGenerator& urng) {
    return Angle<double>::degrees(_distrib(urng));
  }
 private:
  std::normal_distribution<double> _distrib;
};

template <>
class PhysQuantNormalDistrib<Velocity<double> > {
 public:
  PhysQuantNormalDistrib(Velocity<double> mean, Velocity<double> stddev) :
    _distrib(mean.knots(), stddev.knots()) {}

  template<typename UniformRandomNumberGenerator>
  Velocity<double> operator()(UniformRandomNumberGenerator& urng) {
    return Velocity<double>::knots(_distrib(urng));
  }
 private:
  std::normal_distribution<double> _distrib;
};



/*
 * A class that holds corrupted measurements
 * derived from a BoatSim::FullBoatState. Used for testing.
 */
class CorruptedBoatState {
 public:
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


  template <typename T>
  class Corruptor {
   public:
    Corruptor(double scale, T offset, T noiseStd) :
      _distrib(T::zero(), noiseStd), _scale(scale), _offset(offset) {}

    T corrupt(T value, std::default_random_engine &e) {
      return _scale*value + _offset + _distrib(e);
    }
   private:
    double _scale;
    T _offset;
    PhysQuantNormalDistrib<T> _distrib;
  };
 private:
  BoatSim::FullState _trueState;
  Nav _corruptedNav;
};


/*
 * A Testcase holds testdata for a single race that can
 * contain any number of boats. More specific, it holds the following:
 *
 * Common to all boats:
 *   * Local wind conditions in space and time
 *   * Local current conditions in space and time
 *   * A geographic reference point
 *   * A time offset
 *
 * Per boat:
 *   *
 *
 */
class Testcase {
 public:
  typedef GeographicReference::ProjectedPosition ProjectedPosition;

  // Used to represent the local wind/current
  typedef std::function<HorizontalMotion<double>(
      ProjectedPosition, Duration<double>)> FlowFun;


  static FlowFun constantFlowFun(HorizontalMotion<double> m);

  // Description of how the testdata should be synthesized
  // for a boat:
  //  * The dynamics of the boat (BoatCharacteristics)
  //  * TWA-directives for how the boat should steer
  //  * How the measured values are corrupted
  class BoatSpecs {
   public:
    BoatSpecs() {}
    class Dir {
     public:
      Dir() : dur(Duration<double>::seconds(NAN)),
              srcTwa(Angle<double>::degrees(NAN)),
              dstTwa(Angle<double>::degrees(NAN)) {}
      Dir(Duration<double> dur_, Angle<double> srcTwa_, Angle<double> dstTwa_) :
        dur(dur_), srcTwa(srcTwa_), dstTwa(dstTwa_) {}

      Duration<double> dur;
      Angle<double> srcTwa, dstTwa;

      // Used by the twa() method of BoatSimDirs
      Angle<double> interpolate(double localTimeSeconds) const {
        double lambda = localTimeSeconds/dur.seconds();
        return (1.0 - lambda)*srcTwa + lambda*dstTwa;
      }
    };

    BoatSpecs(BoatCharacteristics ch, Array<Dir> dirs);
    Angle<double> twa(Duration<double> dur) const;
   private:
    BoatCharacteristics _ch;
    Array<Dir> _dirs;
    ProportionateIndexer _indexer;
  };


  Testcase(std::default_random_engine &e,
           GeographicReference geoRef,
           TimeStamp timeOffset,
           FlowFun wind,
           FlowFun current,
           Array<BoatSpecs> dirs) :
           _geoRef(geoRef),
           _timeOffset(timeOffset),
           _wind(wind),
           _current(current) {}

  const GeographicReference &geoRef() const {
    return _geoRef;
  }

  Duration<double> toLocalTime(const TimeStamp &x) const {
    return x - _timeOffset;
  }

  TimeStamp fromLocalTime(Duration<double> dur) const {
    return _timeOffset + dur;
  }

  const FlowFun &wind() const {
    return _wind;
  }

  const FlowFun &current() const {
    return _current;
  }

  class BoatData {
   public:

  };
 private:
  GeographicReference _geoRef;
  TimeStamp _timeOffset;
  FlowFun _wind, _current;

  // Simulated boat states over time along with corrupted measurements.
  Array<BoatData> _boatData;
};

} /* namespace mmm */

#endif /* TESTCASE_H_ */
