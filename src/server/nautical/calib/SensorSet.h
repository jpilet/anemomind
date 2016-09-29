/*
 * SensorSet.h
 *
 *  Created on: 1 Sep 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_CALIB_SENSORSET_H_
#define SERVER_NAUTICAL_CALIB_SENSORSET_H_

#include <device/anemobox/Channels.h>
#include <server/math/AxisAngle.h>
#include <Eigen/Dense>
#include <string>
#include <map>

namespace sail {

template <typename T>
using ParamMap = std::map<std::string, T>;

template <typename A, typename B>
void withLookedUpValue(
    const std::map<A, B> &m,
    const A &key, std::function<void(B)> f) {
  auto x = m.find(key);
  if (x != m.end()) {
    f(x->second);
  }
}

// This function is used to cast between different types,
// such as double and ceres::Jet.
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

// This is just a trivial implementation of a parameterized
// object with all methods that we expect there to be, in order
// to make the compiler happy :-). It is
// inherited by DistortionModel and NoiseModel. With template
// specialization, those methods are overridden.
template <typename T>
struct ParameterizedBase {
  typedef T ParameterType;
  static const int paramCount = 0;
  void readFrom(const T *) {}
  void writeTo(T *) const {}
  void readFrom(const ParamMap<T> &) {}
  void writeTo(ParamMap<T> *) const {}
  void outputSummary(std::ostream *) const {}
};

// A distortion model maps a true quantity
// that we are trying to estimate, to an
// observed quantity, to which we want to fit.
// Think about it as the internal camera parameters:
// if they are poorly calibrated, 3d objects will project
// in the wrong place in the image plane.
template <typename T, DataCode code>
struct DistortionModel : ParameterizedBase<T> {};

// This is some kind of robust
// cost function, that we use instead
// of a  square.
template <typename T>
struct GemanMcClure {
  static T apply(T x) {
    T x2 = x*x;
    return x2/(x2 + T(1.0));
  }
};

template <typename T, typename Quantity>
struct MinSigma {};

template <typename T>
struct MinSigma<T, Velocity<T> > {
  static Velocity<T> get() {
    return Velocity<T>::knots(T(0.5));
  }
};

// f(x) = x^2 - y
// f'(x) = 2*x
// Newton-Raphson iteration: x_{i+1} = x_{i} - (x^2 - y)/(2*x)
constexpr double constSqrt(double y, int i, double x) {
  return i <= 0?
    x : constSqrt(y, i-1, x - (x*x - y)/(2.0*x));
}

constexpr double constSqrt(double y) {
  return constSqrt(y, 40, y);
}

/*
 * Unit tests:
 *  Check that the integral is OK
 *
 */
// This is a basic way of applying a cost
template <typename T, typename Quantity>
struct RobustNoiseCost {
  typedef GemanMcClure<T> Rho;
  static const int paramCount = 1;

  static T getInitialScaleParam() {
    constexpr double initialScale = 100.0;
    return T(constSqrt(initialScale - 1));
  }

  // A param that controls how sigma should
  // be scaled. It is mapped to a scaling
  // parameter that is always at least 1.0, so
  // that it is not possible to shrink sigma which
  // would lead to instabilities.
  T scaleParam = getInitialScaleParam();

  // Don't forget to apply the square root on the result
  // of this function, if it is used as a residual in a
  // least-squares solver.
  T apply(Quantity q) const {
    T s2 = scaleParam*scaleParam;
    T scaling = T(1.0) + s2;
    Quantity sigma = scaling*MinSigma<T, Quantity>::get();
    return (Rho::apply(q/sigma) + s2)/(T(1.0) + s2);
  }

  typedef T ParameterType;
  void readFrom(const T *src) {scaleParam = src[0];}
  void writeTo(T *dst) const {dst[0] = scaleParam;}
  void readFrom(const ParamMap<T> &src) {
    withLookedUpValue<std::string, T>(
        src, "scale_param", [&](T x) {
      scaleParam = x;
    });
  }

  void writeTo(ParamMap<T> *dst) const {
    (*dst)["scale_param"] = scaleParam;
  }
  void outputSummary(std::ostream *dst) const {
    *dst << "RobustNoiseCost(scaleParam=" << scaleParam << ")";
  }

};

// This is the base case for noise costs. Nothing at all.
template <typename T, DataCode code>
struct NoiseCost : public ParameterizedBase<T> {};

// All costs that we fit are done in the velocity
// domain, so that we don't mix quantities. But how
// to interpret an angle as a velocity is not the
// concern of this code.
template <typename T>
struct NoiseCost<T, AWA> :
  public RobustNoiseCost<T, Velocity<T> > {};

template <typename T>
struct NoiseCost<T, AWS> :
  public RobustNoiseCost<T, Velocity<T> > {};

template <typename T>
struct NoiseCost<T, WAT_SPEED> :
  public RobustNoiseCost<T, Velocity<T> > {};

template <typename T>
struct NoiseCost<T, MAG_HEADING> :
  public RobustNoiseCost<T, Velocity<T> > {};

// This object groups a distortion model with a noise cost.
template <typename T, DataCode code>
struct SensorModel {
  typedef DistortionModel<T, code> TDistortionModel;
  typedef NoiseCost<T, code> TNoiseCost;

  // How the true quantity maps to a distorted quantity.
  TDistortionModel dist;

  // How we penalize deviations between an observation
  // and our estimate when optimizing
  TNoiseCost noiseCost;

  typedef T ParameterType;
  static const int paramCount =
      TDistortionModel::paramCount +
      TNoiseCost::paramCount;

  void readFrom(const T *x) {
    dist.readFrom(x);
    noiseCost.readFrom(x + TDistortionModel::paramCount);
  }

  void writeTo(T *x) const {
    dist.writeTo(x);
    noiseCost.writeTo(x + TDistortionModel::paramCount);
  }

  void readFrom(const ParamMap<ParamMap<T> > &x) {
    withLookedUpValue<std::string, ParamMap<T>>(
        x, "dist", [&](const ParamMap<T> &m) {
      dist.readFrom(m);
    });
    withLookedUpValue<std::string, ParamMap<T>>(
        x, "noiseCost",
        [&](const ParamMap<T> &m) {
      noiseCost.readFrom(m);
    });
  }

  void writeTo(ParamMap<ParamMap<T> > *x) const {
    dist.writeTo(&((*x)["dist"]));
    noiseCost.writeTo(&((*x)["noiseCost"]));
  }

  void outputSummary(std::ostream *dst) const {
    *dst << "SensorModel(dist=";
    dist.outputSummary(dst);
    *dst << ", noiseCost=";
    noiseCost.outputSummary(dst);
    *dst <<")";
  }
};

template <typename T>
class BasicAngleSensor {
public:
  typedef T ParameterType;
  BasicAngleSensor() : _offset(Angle<T>::radians(T(0.0))) {}

  static const int paramCount = 1;

  void readFrom(const T *src) {
    _offset = Angle<T>::radians(src[0]);
  }

  void writeTo(T *dst) const {
    dst[0] = _offset.radians();
  }

  void readFrom(const ParamMap<T> &src) {
    auto f = src.find("offset-radians");
    if (f != src.end()) {
      _offset = Angle<T>::radians(f->second);
    }
  }

  void writeTo(ParamMap<T> *dst) const {
    (*dst)["offset-radians"] = _offset.radians();
  }

  void outputSummary(std::ostream *dst) const {
    *dst << "BasicAngleSensor(offset="
        << _offset.radians() << " radians)";
  }

  Angle<T> offset() const {return _offset;}

  Angle<T> apply(Angle<T> x) const {
    return x + _offset;
  }

  HorizontalMotion<T> apply(const HorizontalMotion<T> &x) const {
    return x.rotate(_offset);
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

  void readFrom(const T *src) {_bias = src[0];}
  void writeTo(T *dst) const {dst[0] = _bias;}

  void outputSummary(std::ostream *dst) const {
    *dst << "BasicSpeedSensor1(bias=" << _bias << ")";
  }

  void readFrom(const ParamMap<T> &src) {
    auto f = src.find("bias");
    if (f != src.end()) {
      _bias = f->second;
    }
  }

  void writeTo(ParamMap<T> *dst) const {
    (*dst)["bias"] = _bias;
  }

  Velocity<T> apply(Velocity<T> x) const {
    return _bias*x;
  }

  T bias() const {return _bias;}
private:
  T _bias;
};

/*
 * Orientation formulation:
 *  The IMU measures a rotation from coordinate system of the sensor
 *  to the world, R_imu
 *
 *  So if we have a vector X_s in sensor coordinates, we get its world
 *  coordinates as
 *  X_w = R_imu*X_s
 *
 *  But we like to work with coordinates X_b in the coordinate system of
 *  the boat. So we need another rotation R_c that maps those coordinates
 *  to the box:
 *
 *  X_s = R_c*X_b
 *  and then
 *  X_w = R_imu*X_s
 *
 *  that we can wrap up as
 *  X_w = R_imu*R_c*X_c
 *
 *  In other words, the full rotation from the boat to the world is
 *
 *  R = R_imu*R_c
 *
 *  We can think of the R_c matrix as the calibration of the sensor.
 *  We choose to use the compact axis-angle representation to represent
 *  that matrix.
 *
 */
template <typename T>
class OrientationSensor {
public:
  typedef T ParameterType;

  Eigen::Matrix<T, 3, 3> boatToSensorRotation() const {
    return computeRotationFromOmega(_omega);
  }

  Eigen::Matrix<T, 3, 3> computeBoatToWorldRotation( // this is what 'apply' is for the other sensors...
      const Eigen::Matrix<T, 3, 3> &sensorToWorld) const {
    return sensorToWorld*boatToSensorRotation();
  }

  OrientationSensor() : _omega(T(0.0), T(0.0), T(0.0)) {}

  static const int paramCount = 3;

  void readFrom(const T *src) {
    for (int i = 0; i < paramCount; i++) {
      _omega(i) = src[i];
    }
  }

  void writeTo(T *dst) const {
    for (int i = 0; i < paramCount; i++) {
      dst[i] = _omega(i);
    }
  }

  void readFrom(const ParamMap<T> &src) {
    const char keys[3] = {'x', 'y', 'z'};
    for (int i = 0; i < paramCount; i++) {
      auto f = src.find(std::string("omega-") + keys[i] + "-radians");
      if (f != src.end()) {
        _omega(i) = f->second;
      }
    }

  }

  void writeTo(ParamMap<T> *dst) const {
    (*dst)["omega-x-radians"] = _omega(0);
    (*dst)["omega-y-radians"] = _omega(1);
    (*dst)["omega-z-radians"] = _omega(2);
  }

  void outputSummary(std::ostream *dst) const {
    *dst << "OrientationSensor(omega="
        << _omega.transpose() << ")";
  }

  Eigen::Matrix<T, 3, 1> omega() const {
    return _omega;
  }
private:
  Eigen::Matrix<T, 3, 1> _omega;
};

template <typename T>
struct DistortionModel<T, AWA> :
  public BasicAngleSensor<T> {};

template <typename T>
struct DistortionModel<T, MAG_HEADING> :
  public BasicAngleSensor<T> {};

template <typename T>
struct DistortionModel<T, AWS> :
  public BasicSpeedSensor1<T> {};

template <typename T>
struct DistortionModel<T, WAT_SPEED> :
  public BasicSpeedSensor1<T> {};

template <typename T>
struct DistortionModel<T, ORIENT> :
  public OrientationSensor<T>{};

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
  const T *src;

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
    if (!obj.empty()) {
      *dst << "  Sensors(code=" << wordIdentifierForCode(code) << ",\n";
      for (const auto &kv: obj) {
        *dst << "    " << kv.first << "=";
        kv.second.outputSummary(dst);
        *dst << std::endl;
      }
      *dst << "  )\n";
    }
  }
};


// DataCode, source channel, field in SensorModel, Parameter in model
template <typename T>
using SensorParameterMap =
    std::map<DataCode, ParamMap<ParamMap<ParamMap<T>>>>;



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
    auto &dstSub = (*dst)[code];
    for (const auto &kv: obj) {
      auto &sub = dstSub[kv.first];
      kv.second.writeTo(&sub);
    }
  }
};

enum class FunctionCode {
  Noise,
  Distortion,
  NoiseAndDistortion
};

template <FunctionCode FunctionType, typename T>
struct SensorFunctionSet;

template <FunctionCode FType, typename DstType>
struct SensorSetCaster {
  typedef SensorFunctionSet<FType, DstType> DstSensorSet;

  DstSensorSet result;

  template <DataCode code, typename X, typename SensorModelMap>
    void visit(const SensorModelMap &obj) {
    auto &sub = *(ChannelFieldAccess<code>::template get<DstSensorSet>(result));
    for (const auto &kv: obj) {
      castSensorParameters(kv.second, &(sub[kv.first]));
    }
  }
};

template <enum FunctionCode, typename T, DataCode code>
struct SensorFunctionType {};

template <typename T, DataCode code>
struct SensorFunctionType<FunctionCode::Noise, T, code> {
  typedef NoiseCost<T, code> type;
};

template <typename T, DataCode code>
struct SensorFunctionType<FunctionCode::Distortion, T, code> {
  typedef DistortionModel<T, code> type;
};

template <typename T, DataCode code>
struct SensorFunctionType<
  FunctionCode::NoiseAndDistortion, T, code> {
  typedef SensorModel<T, code> type;
};


// Model for all the sensors on the boat.
template <FunctionCode FunctionType, typename T>
struct SensorFunctionSet {
#define MAKE_SENSOR_FIELD(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  std::map<std::string, typename SensorFunctionType<FunctionType, T, HANDLE>::type> HANDLE;
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
  void readFrom(const T *src) {
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
  void outputSummary(std::ostream *dst) const {
    SensorSetSummaryVisitor v{dst};
    visitFieldsConst(*this, &v);
  }

  // Useful in the objective function,
  // where we want to construct a SensorSet that
  // is automatically differentiable.
  template <typename DstType>
  SensorFunctionSet<FunctionType, DstType> cast() {
    SensorSetCaster<FunctionType, DstType> caster;
    visitFieldsConst(*this, &caster);
    return caster.result;
  }
};

template <typename T>
using SensorSet =
    SensorFunctionSet<FunctionCode::NoiseAndDistortion, T>;

template <typename T>
using SensorNoiseSet =
    SensorFunctionSet<FunctionCode::Noise, T>;

template <typename T>
using SensorDistortionSet =
    SensorFunctionSet<FunctionCode::Distortion, T>;

} /* namespace sail */

#endif /* SERVER_NAUTICAL_CALIB_FITNESS_H_ */
