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

  double mapToReal(TimeStamp t) const {
    return (t - _offset)/_period;
  }

  int mapUnbounded(TimeStamp t) const {
    return int(round(mapToReal(t)));
  }

  int map(TimeStamp t) const {
    int index = mapUnbounded(t);
    return 0 <= index && index < _sampleCount? index : -1;
  }

  TimeStamp unmap(double i) const {
    return _offset + i*_period;
  }

  int firstSampleIndex() const {return 0;}
  int lastSampleIndex() const {return _sampleCount-1;}
  TimeStamp firstSampleTime() const {return unmap(firstSampleIndex());}
  TimeStamp lastSampleTime() const {return unmap(lastSampleIndex());}

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
