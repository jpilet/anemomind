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
#include <random>

namespace sail {

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

  // Boat-specific data for a simulation
  class BoatSimDirs {
   public:
    class Dir {
     public:
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

    BoatSimDirs(BoatCharacteristics ch, Array<Dir> dirs);
    Angle<double> twa(Duration<double> dur) const;
   private:
    BoatCharacteristics _ch;
    Array<Dir> _dirs;
    ProportionateIndexer _indexer;
  };


  Testcase(std::default_random_engine &e,
           GeographicReference geoRef,
           TimeStamp timeOffset,
           FlowFun wind, FlowFun current,
           Array<BoatSimDirs> dirs) :
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
 private:
  GeographicReference _geoRef;
  TimeStamp _timeOffset;
  FlowFun _wind, _current;
};

} /* namespace mmm */

#endif /* TESTCASE_H_ */
