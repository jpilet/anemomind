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

template <typename Src, typename Dst>
void castSensorParameters(const Src &src, Dst *dst) {
  constexpr int N = Src::paramCount;
  static_assert(
      N == Dst::paramCount,
      "Incompatible parameters");

  typedef typename Src::ParameterType SrcT;
  typedef typename Dst::ParameterType DstT;
  SrcT srcX[N];
  DstT dstX[N];
  src.writeTo(srcX);
  for (int i = 0; i < N; i++) {
    dstX[i] = DstT(srcX[i]);
  }
  dst->readFrom(dstX);
}

template <typename T, DataCode code>
struct SensorModel {
  typedef T ParameterType;
  static const int paramCount = 0;
  void readFrom(T *) {}
  void writeTo(T *) const {}
  void readFrom(const std::map<std::string, T> &) {}
  void writeTo(std::map<std::string, T> *) const {}
  void outputSummary(std::ostream *) const {}
};

template <typename T>
class BasicAngleSensor {
public:
  typedef T ParameterType;
  BasicAngleSensor() : _offset(Angle<T>::radians(T(0.0))) {}

  static const int paramCount = 1;

  void readFrom(T *src) {
    _offset = Angle<T>::radians(src[0]);
  }

  void writeTo(T *dst) const {
    dst[0] = _offset.radians();
  }

  void readFrom(const std::map<std::string, T> &src) {
    auto f = src.find("offset-radians");
    if (f != src.end()) {
      _offset = Angle<T>::radians(f->second);
    }
  }

  void writeTo(std::map<std::string, T> *dst) const {
    (*dst)["offset-radians"] = _offset.radians();
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
  typedef T ParameterType;
  BasicSpeedSensor1() : _bias(T(1.0)) {}

  static const int paramCount = 1;

  void readFrom(T *src) {_bias = src[0];}
  void writeTo(T *dst) const {dst[0] = _bias;}

  void outputSummary(std::ostream *dst) const {
    *dst << "BasicSpeedSensor1 with bias " << _bias;
  }

  void readFrom(const std::map<std::string, T> &src) {
    auto f = src.find("bias");
    if (f != src.end()) {
      _bias = f->second;
    }
  }

  void writeTo(std::map<std::string, T> *dst) const {
    (*dst)["bias"] = _bias;
  }

private:
  T _bias;
};

template <typename T>
struct SensorModel<T, AWA> :
  public BasicAngleSensor<T> {};

template <typename T>
struct SensorModel<T, MAG_HEADING> :
  public BasicAngleSensor<T> {};

template <typename T>
struct SensorModel<T, AWS> :
  public BasicSpeedSensor1<T> {};

template <typename T>
struct SensorModel<T, WAT_SPEED> :
  public BasicSpeedSensor1<T> {};


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
  void visit(SensorModelMap *obj) {
    for (auto &kv: *obj) {
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


template <typename T>
using SensorParameterMap = std::map<DataCode,
    std::map<std::string, std::map<std::string, T>>>;

template <typename T>
struct SensorSetParamMapReader {
  const SensorParameterMap<T> &src;

  template <DataCode code, typename X, typename SensorModelMap>
  void visit(SensorModelMap *obj) {
    auto f0 = src.find(code);
    if (f0 != src.end()) {
      const auto &sub = f0->second;
      for (auto &kv: *obj) {
        auto f = sub.find(kv.first);
        if (f != sub.end()) {
          kv.second.readFrom(f->second);
        }
      }
    }
  }
};

template <typename T>
struct SensorSetParamMapWriter {
  SensorParameterMap<T> *dst;

  template <DataCode code, typename X, typename SensorModelMap>
  void visit(const SensorModelMap &obj) {
    std::map<std::string, std::map<std::string, T>> &dstSub = (*dst)[code];
    for (const auto &kv: obj) {
      std::map<std::string, T> &sub = dstSub[kv.first];
      kv.second.writeTo(&sub);
    }
  }
};

template <typename T>
struct SensorSet;

template <typename DstType>
struct SensorSetCaster {
  typedef SensorSet<DstType> DstSensorSet;

  DstSensorSet result;

  template <DataCode code, typename X, typename SensorModelMap>
    void visit(const SensorModelMap &obj) {
    auto &sub = *(ChannelFieldAccess<code>::template get<DstSensorSet>(result));
    for (const auto &kv: obj) {
      castSensorParameters(kv.second, &(sub[kv.first]));
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

  // Useful for determining the length of the vector
  // vector of parameters to be optimized.
  int paramCount() const {
    SensorSetParamCounter counter;
    visitFieldsConst(*this, &counter);
    return counter.counter;
  }

  // Useful when reading the parameters to be optimized
  // from the input argument in the objective function.
  void readFrom(T *src) {
    SensorSetParamReader<T> reader{src};
    visitFieldsMutable(this, &reader);
  }

  // Useful when initializing the X vector of unknowns
  // to be optimized.
  void writeTo(T *dst) const {
    SensorSetParamWriter<T> writer{dst};
    visitFieldsConst(*this, &writer);
  }

  // Useful when reading parameters from some
  // data structure, such as a json map
  void readFrom(const SensorParameterMap<T> &src) {
    SensorSetParamMapReader<T> reader{src};
    visitFieldsMutable(this, &reader);
  }

  // Useful when converting the parameters
  // to something that can be easily stored, e.g. in a
  // json map.
  void writeTo(SensorParameterMap<T> *dst) const {
    SensorSetParamMapWriter<T> writer{dst};
    visitFieldsConst(*this, &writer);
  }

  // Useful for debugging.
  void outputSummary(std::ostream *dst) {
    SensorSetSummaryVisitor v{dst};
    visitFieldsConst(*this, &v);
  }

  // Useful in the objective function,
  // where we want to construct a SensorSet that
  // is automatically differentiable.
  template <typename DstType>
  SensorSet<DstType> cast() {
    SensorSetCaster<DstType> caster;
    visitFieldsConst(*this, &caster);
    return caster.result;
  }
};

} /* namespace sail */

#endif /* SERVER_NAUTICAL_CALIB_FITNESS_H_ */
