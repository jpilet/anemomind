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
  void readFrom(T *) {}
  void writeTo(T *) const {}
  void outputSummary(std::ostream *) const {}
};

template <typename T>
class BasicAngleSensor {
public:
  BasicAngleSensor() : _offset(Angle<T>::radians(T(0.0))) {
    std::cout << "///////////////" << std::endl;
  }

  static const int paramCount = 1;

  void readFrom(T *src) {
    std::cout << "REad value: " << src[0] << std::endl;
    _offset = Angle<T>::radians(src[0]);
    std::cout << "Read this: " << _offset.radians() << std::endl;
  }

  void writeTo(T *dst) const {
    dst[0] = _offset.radians();
  }

  void outputSummary(std::ostream *dst) const {
    *dst << "BasicAngleSensor with offset "
        << _offset.radians() << " radians";
  }
private:
  Angle<T> _offset;
};

template <typename T>
class BasicSpeedSensor1 {
public:
  BasicSpeedSensor1() : _bias(T(1.0)) {}

  static const int paramCount = 1;

  void readFrom(T *src) {_bias = src[0];}
  void writeTo(T *dst) const {dst[0] = _bias;}

  void outputSummary(std::ostream *dst) const {
    *dst << "BasicSpeedSensor1 with bias " << _bias;
  }
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
  void visit(SensorModelMap &obj) {
    for (auto &kv: obj) {
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
    for (const auto &kv: obj) {
      kv.second.writeTo(dst);
      dst += kv.second.paramCount;
    }
  }
};

struct SensorSetSummaryVisitor {
  std::ostream *dst;

  template <DataCode code, typename X, typename SensorModelMap>
  void visit(const SensorModelMap &obj) {
    *dst << "Sensors for " << wordIdentifierForCode(code) << "\n";
    for (const auto &kv: obj) {
      *dst << "  " << kv.first << ": ";
      kv.second.outputSummary(dst);
      *dst << std::endl;
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
    visitFieldsConst(*this, &counter);
    return counter.counter;
  }

  // Useful when reading the parameters to be optimized
  void readFrom(T *src) {
    SensorSetParamReader<T> reader{src};
    visitFieldsMutable(this, &reader);
  }

  void writeTo(T *dst) const {
    SensorSetParamWriter<T> writer{dst};
    visitFieldsConst(*this, &writer);
  }

  void outputSummary(std::ostream *dst) {
    SensorSetSummaryVisitor v{dst};
    visitFieldsConst(*this, &v);
  }
};

} /* namespace sail */

#endif /* SERVER_NAUTICAL_CALIB_FITNESS_H_ */
