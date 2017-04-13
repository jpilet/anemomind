/*
 * TimeMapper.h
 *
 *  Created on: 31 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_TIMEMAPPER_H_
#define SERVER_COMMON_TIMEMAPPER_H_

#include <server/common/TimeStamp.h>

namespace sail {

// Associates sample indices to a time line.
// Useful when optimizing samples of a temporal signal.
class TimeMapper {
public:

  bool empty() const {
    return 0 == _sampleCount;
  }

  Duration<double> totalDuration() const {
    return double(_sampleCount)*_period;
  }

  TimeMapper() : _sampleCount(0) {}
  TimeMapper(TimeStamp offs, Duration<double> per,
      int n) : _offset(offs), _period(per), _sampleCount(n) {}

  double toRealIndex(TimeStamp t) const {
    return (t - _offset)/_period;
  }

  int toIntegerIndex(TimeStamp t) const {
    return int(round(toRealIndex(t)));
  }

  int toBoundedIntegerIndex(TimeStamp t) const {
    int index = toIntegerIndex(t);
    return 0 <= index && index < _sampleCount? index : -1;
  }

  TimeStamp toTimeStamp(double i) const {
    return _offset + i*_period;
  }

  int firstSampleIndex() const {return 0;}
  int lastSampleIndex() const {return _sampleCount-1;}
  TimeStamp firstSampleTime() const {return toTimeStamp(firstSampleIndex());}
  TimeStamp lastSampleTime() const {return toTimeStamp(lastSampleIndex());}

  TimeStamp offset() const {return _offset;}
  Duration<double> period() const {return _period;}
  int sampleCount() const {return _sampleCount;}
private:
  TimeStamp _offset;
  Duration<double> _period;
  int _sampleCount = 0;
};

}

#endif /* SERVER_COMMON_TIMEMAPPER_H_ */
