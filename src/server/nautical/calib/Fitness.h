/*
 * Fitness.h
 *
 *  Created on: 26 Aug 2016
 *      Author: jonas
 *
 * This code is about generating the residuals for fitting to data.
 */

#ifndef SERVER_NAUTICAL_CALIB_FITNESS_H_
#define SERVER_NAUTICAL_CALIB_FITNESS_H_

#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/DispatcherUtils.h>

namespace sail {


template <typename T, DataCode code>
struct SensorModel {
  static const int paramCount = 0;
  void readFrom(T *) const {}
  void writeTo(T *) const {}
};

template <typename T>
class BasicAngleSensor {
public:
  BasicAngleSensor() : _offset(Angle<T>::radians(T(0.0))) {}
  static const int paramCount = 1;
  void readFrom(T *src) const {_offset = Angle<T>::radians(src[0]);}
  void writeTo(T *dst) const {dst[0] = _offset.radians();}
private:
  Angle<T> _offset;
};

template <typename T>
class BasicSpeedSensor1 {
public:
  BasicSpeedSensor1() : _bias(T(1.0)) {}
  static const int paramCount = 1;
  void readFrom(T *src) const {_bias = src[0];}
  void writeTo(T *dst) const {dst[0] = _bias;}
private:
  T _bias;
};

template <typename T>
struct SensorModel<T, AWA> : public BasicAngleSensor<T> {};

template <typename T>
struct SensorModel<T, MAG_HEADING> : public BasicAngleSensor<T> {};

template <typename T>
struct SensorModel<T, AWS> : public BasicSpeedSensor1<T> {};

template <typename T>
struct SensorModel<T, WAT_SPEED> : public BasicSpeedSensor1<T> {};


struct SensorSetParamCounter {
  int counter = 0;
  template <DataCode code, typename T, typename SensorModelMap>
  void visit(const SensorModelMap &obj) {
    for (auto kv: obj) {
      counter += kv.second.paramCount;
    }
  }
};

template <typename T>
struct SensorSetParamReader {
  T *src;

  template <DataCode code, typename X, typename SensorModelMap>
  void visit(const SensorModelMap &obj) {
    for (auto kv: obj) {
      kv.second.readFrom(src);
      src += kv.second.paramCount;
    }
  }
};

template <typename T>
struct SensorSetParamWriter {
  T *dst;

  template <DataCode code, typename X, typename SensorModelMap>
  void visit(const SensorModelMap &obj) {
    for (auto kv: obj) {
      kv.second.writeTo(dst);
      dst += kv.second.paramCount;
    }
  }
};

// Model for all the sensors on the boat.
template <typename T>
struct SensorSet {
#define MAKE_SENSOR_FIELD(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  std::map<std::string, SensorModel<T, HANDLE> > HANDLE;
FOREACH_CHANNEL(MAKE_SENSOR_FIELD)
#undef MAKE_SENSOR_FIELD

  int paramCount() const {
    SensorSetParamCounter counter;
    visitFields(*this, &counter);
    return counter.counter;
  }

  // Useful when reading the parameters to be optimized
  void readFrom(T *src) {
    SensorSetParamReader<T> reader{src};
    visitFields(*this, &reader);
  }

  void writeTo(T *dst) {
    SensorSetParamWriter<T> writer{dst};
    visitFields(*this, &writer);
  }
};

} /* namespace sail */

#endif /* SERVER_NAUTICAL_CALIB_FITNESS_H_ */
