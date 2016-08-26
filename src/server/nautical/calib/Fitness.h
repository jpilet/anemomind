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


struct ParamCounter {
  int counter = 0;
  template <DataCode code, typename T, typename Obj>
  void visit(const Obj &obj) {
    for (auto kv: obj) {
      counter += kv.second.paramCount;
    }
  }
};

template <typename Target, typename Visitor>
void visitFields(const Target &target, Visitor *v) {
#define VISIT_FIELD(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  v->template visit<HANDLE, TYPE, decltype(target.HANDLE)>(target.HANDLE);
  FOREACH_CHANNEL(VISIT_FIELD)
#undef VISIT_FIELD
}

// Model for all the sensors on the boat.
template <typename T>
struct SensorSet {
#define MAKE_SENSOR_FIELD(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  std::map<std::string, SensorModel<T, HANDLE> > HANDLE;
FOREACH_CHANNEL(MAKE_SENSOR_FIELD)
#undef MAKE_SENSOR_FIELD

  int paramCount() const {
    ParamCounter counter;
    visitFields(*this, &counter);
    return counter.counter;
  }
};

} /* namespace sail */

#endif /* SERVER_NAUTICAL_CALIB_FITNESS_H_ */
