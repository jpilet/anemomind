#ifndef ANEMOBOX_DISPATCHER_H
#define ANEMOBOX_DISPATCHER_H

// The main class of this file is 'Dispatcher'.
// The dispatcher receive data from one or more DataSource, and publish it
// to consumers that can subscribe to any value.

#include <boost/signals2/signal.hpp>
#include <map>
#include <memory>
#include <string>

#include <device/anemobox/BinarySignal.h>
#include <device/anemobox/ValueDispatcher.h>
#include <server/common/string.h>

namespace sail {

/*
 List of channels used by Dispatcher and NavHistory.

 This macro provides an easy way to iterate over channels at compile time.

 To use this macro, define a macro that takes the following arguments:
  #define ENUM_ENTRY(handle, code, shortname, type, description)
 
 For example, here's how to declare a switch for each entry:

 #define CASE_ENTRY(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
     case handle : return shortname;
 
   FOREACH_CHANNEL(CASE_ENTRY)
 #undef CASE_ENTRY

 Rules to follow for modifying this list:
 - never remove any entry
 - never change an existing shortname
 - never change an existing code
 otherwise this might break compatibility with recorded data.
*/
#define FOREACH_CHANNEL(X) \
  X(AWA, 1, "awa", Angle<>, "apparent wind angle") \
  X(AWS, 2, "aws", Velocity<>, "apparent wind speed") \
  X(TWA, 3, "twa", Angle<>, "true wind angle") \
  X(TWS, 4, "tws", Velocity<>, "true wind speed") \
  X(TWDIR, 5, "twdir", Angle<>, "true wind direction") \
  X(GPS_SPEED, 6, "gpsSpeed", Velocity<>, "GPS speed") \
  X(GPS_BEARING, 7, "gpsBearing", Angle<>, "GPS bearing") \
  X(MAG_HEADING, 8, "magHdg", Angle<>, "magnetic heading") \
  X(WAT_SPEED, 9, "watSpeed", Velocity<>, "water speed") \
  X(WAT_DIST, 10, "watDist", Length<>, "distance over water") \
  X(GPS_POS, 11, "pos", GeographicPosition<double>, "GPS position") \
  X(DATE_TIME, 12, "dateTime", TimeStamp, "GPS date and time (UTC)") \
  X(TARGET_VMG, 13, "targetVmg", Velocity<>, "Target VMG") \
  X(VMG, 14, "vmg", Velocity<>, "VMG") \
  X(ORIENT, 15, "orient", AbsoluteOrientation, "Absolute anemobox orientation") \
  X(RUDDER_ANGLE, 16, "rudderAngle", Angle<>, "Rudder angle") \
  X(VALID_GPS, 17, "validGps", BinaryEdge, "Valid GPS periods") \
  X(RATE_OF_TURN, 18, "rot", AngularVelocity<>, "Rate of turn") \
  X(TOT_WAT_DIST, 19, "totDist", Length<>, "total distance over water") \
  X(ENGINE_RPM, 20, "rpm", AngularVelocity<>, "Engine RPM") \
  X(YAW, 21, "yaw", Angle<>, "Yaw (heading)") \
  X(PITCH, 22, "pitch", Angle<>, "Pitch") \
  X(ROLL, 23, "roll", Angle<>, "Roll")

enum DataCode {
#define ENUM_ENTRY(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  HANDLE = CODE,

  FOREACH_CHANNEL(ENUM_ENTRY)
#undef ENUM_ENTRY
};

template <DataCode Code> struct TypeForCode { };

const std::vector<DataCode>& allDataCodes();

#define DECL_TYPE(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
template<> struct TypeForCode<HANDLE> { typedef TYPE type; };
FOREACH_CHANNEL(DECL_TYPE)
#undef DECL_TYPE

const char* descriptionForCode(DataCode code);
const char* wordIdentifierForCode(DataCode code);

template <typename T> class TypedDispatchData;
class DispatchDataVisitor;
class DispatchData {
 public:
  DispatchData(DataCode code, std::string source) : _source(source), _code(code) { }

  const char* description() const { return descriptionForCode(_code); }
  DataCode dataCode() const { return _code; }
  const std::string& source() const { return _source; }


  //! returns a single word that describe what this value is.
  // For example: awa, tws, watSpeed, etc.
  std::string wordIdentifier() const { return wordIdentifierForCode(_code); }

  virtual bool isFresh(Duration<> maxAge = Duration<>::seconds(15)) const = 0;

  virtual void visit(DispatchDataVisitor *visitor) = 0;

  // Templated version of the above thing
  template <typename X>
  void visitX(X* x);

  virtual ~DispatchData() {}

 protected:
  std::string _source;
 private:
  DataCode _code;
};

template <typename T>
class TypedDispatchData : public DispatchData {
 public:
  TypedDispatchData(DataCode nature, std::string source)
     : DispatchData(nature, source) { }

  virtual void visit(DispatchDataVisitor *visitor);
  virtual ValueDispatcher<T> *dispatcher() = 0;
  virtual const ValueDispatcher<T> *dispatcher() const = 0;
  virtual void setValue(T value) = 0;
  virtual bool isFresh(Duration<> maxAge = Duration<>::seconds(15)) const {
    auto d = dispatcher();
    if (d == nullptr || !d->hasValue() || !d->clock()) {
      return false;
    }
    Duration<> delta(d->clock()->currentTime() - d->lastTimeStamp()) ;
    return delta < maxAge;
  }
};

template <typename X>
void DispatchData::visitX(X* x) {
  switch (_code) {
#define VISIT_X(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  case HANDLE: {typedef TypedDispatchData<TYPE> T; x->template visit<TYPE>( \
    HANDLE, dynamic_cast<T*>(this)); break;}
FOREACH_CHANNEL(VISIT_X)
#undef VISIT_X
  default: break;
  }
}

namespace {
  static const int defaultDispatcherBufferLength = 1024;
}

template <typename T>
class TypedDispatchDataReal : public TypedDispatchData<T> {
 public:
  TypedDispatchDataReal(DataCode nature,
                        std::string source,
                        Clock* clock, int bufferLength)
     : TypedDispatchData<T>(nature, source),
     _dispatcher(clock, bufferLength) { }

  virtual ValueDispatcher<T> *dispatcher() { return &_dispatcher; }
  virtual const ValueDispatcher<T> *dispatcher() const { return &_dispatcher; }

  virtual void setValue(T value) { _dispatcher.setValue(value); }
  virtual ~TypedDispatchDataReal() {}
 private:
  ValueDispatcher<T> _dispatcher;
};
typedef TypedDispatchData<Angle<double>> DispatchAngleData;
typedef TypedDispatchData<Velocity<double>> DispatchVelocityData;
typedef TypedDispatchData<Length<double>> DispatchLengthData;
typedef TypedDispatchData<GeographicPosition<double>> DispatchGeoPosData;
typedef TypedDispatchData<TimeStamp> DispatchTimeStampData;
typedef TypedDispatchData<AbsoluteOrientation> DispatchAbsoluteOrientationData;
typedef TypedDispatchData<BinaryEdge> DispatchBinaryEdge;
typedef TypedDispatchData<AngularVelocity<double>> DispatchAngularVelocityData;

template <typename T>
class DispatchDataProxy : public TypedDispatchData<T> {
 public:
   DispatchDataProxy(DataCode code) : TypedDispatchData<T>(code, ""),
     _forward(nullptr) { }

  virtual ValueDispatcher<T> *dispatcher() { return &_proxy; }
  virtual const ValueDispatcher<T> *dispatcher() const {
    return &_proxy;
  }

  void setActiveDispatcher(TypedDispatchData<T>* dispatcher) {
    this->_source = dispatcher->source();
    this->_forward = dispatcher;
    _proxy.proxy(dispatcher->dispatcher());
  }

  bool hasDispatcher() const { return _proxy.hasDispatcher(); }
  TypedDispatchData<T> *realDispatcher() const { return _forward; }

  virtual void setValue(T value) { _proxy.setValue(value); }
  virtual ~DispatchDataProxy() {}
 private:
  ValueDispatcherProxy<T> _proxy;
  TypedDispatchData<T>* _forward;
};

class DispatchDataVisitor {
 public:
  virtual void run(DispatchAngleData *angle) = 0;
  virtual void run(DispatchVelocityData *velocity) = 0;
  virtual void run(DispatchLengthData *length) = 0;
  virtual void run(DispatchGeoPosData *pos) = 0;
  virtual void run(DispatchTimeStampData *timestamp) = 0;
  virtual void run(DispatchAbsoluteOrientationData *orient) = 0;
  virtual void run(DispatchBinaryEdge *data) = 0;
  virtual void run(DispatchAngularVelocityData *data) = 0;
  virtual ~DispatchDataVisitor() {}
};

template <typename T>
void TypedDispatchData<T>::visit(DispatchDataVisitor *visitor) {
  visitor->run(this);
}

// Warning: We just borrow the pointer
// returned by this function. We are not
// allowed to wrap inside a std::shared_ptr
// or delete it.
template <DataCode Code>
TypedDispatchData<typename TypeForCode<Code>::type>* toTypedDispatchData(DispatchData *data) {
  return dynamic_cast<TypedDispatchData<typename TypeForCode<Code>::type>*>(data);
}

//! Dispatcher: the hub for all values processed by the anemobox.
// the data() method allows enumeration of all components.
class Dispatcher : public Clock {
 public:
  Dispatcher();
  static const int defaultPriority = 0;

  const std::map<DataCode, std::shared_ptr<DispatchData>>& dispatchers()
    const { return _currentSource; }

  const std::map<DataCode,
    std::map<std::string,
      std::shared_ptr<DispatchData>>> &allSources() const {
    return _data;
  }

  // Returns null if not exist
  std::shared_ptr<DispatchData> dispatchDataForSource(DataCode code, const std::string& source) const;

  virtual int maxBufferLength() const {return defaultDispatcherBufferLength;}

  // Return or create a DispatchData for the given source.
  template <typename T>
  TypedDispatchData<T>* createDispatchDataForSource(
      DataCode code, const std::string& source, int size);

  DispatchData* dispatchData(DataCode code) const {
    auto it = _currentSource.find(code);
    assert (it != _currentSource.end());
    return it->second.get();
  }

  bool has(DataCode c) const {
    auto found = _data.find(c);
    if (found == _data.end()) {
      return false;
    }
    return !found->second.empty();
  }

  template <DataCode Code>
  TypedDispatchData<typename TypeForCode<Code>::type>* get() const {
    return toTypedDispatchData<Code>(dispatchData(Code));
  }

  template <DataCode Code>
  TypedDispatchData<typename TypeForCode<Code>::type>* get(
      const std::string& source) const {
    return toTypedDispatchData<Code>(dispatchDataForSource(Code, source).get());
  }


  template <DataCode Code>
  const TimedSampleCollection<typename TypeForCode<Code>::type> &values() const {
    return get<Code>()->dispatcher()->values();
  }

  template <DataCode Code>
  const TimedSampleCollection<typename TypeForCode<Code>::type> &values(
      const std::string& source) const {
    return get<Code>(source)->dispatcher()->values();
  }

  // Temporary method when treating it as a dataset.
  // First, try to get samples from the source with highest priority, if there is at least one sample.
  // Otherwise, try to get samples from any other non-empty source.
  // TODO: Do something more sophisticated than this, later.
  template <DataCode Code>
  const TimedSampleCollection<typename TypeForCode<Code>::type> &getNonEmptyValues() const {
    const auto &v = values<Code>();

    if (!v.empty()) {
      return v;
    }

    auto sources = _data.find(Code);
    if (sources != _data.end()) {
      for (auto kv: sources->second) {
        const auto &x = toTypedDispatchData<Code>(kv.second.get())->dispatcher()->values();
        if (!x.empty()) {
          return x;
        }
      }
    }
    // All were empty :-( Return v anyway.
    return v;
  }

  template <DataCode Code>
  typename TypeForCode<Code>::type val() {
    return get<Code>()->dispatcher()->lastValue();
  }

  int onNewSource(std::function<void(DataCode, const std::string&)> f);
  void removeNewSourceListener(int);

  template <typename T>
  void publishValue(DataCode code, const std::string& source, T value) {
    TypedDispatchData<T>* dispatchData =
      createDispatchDataForSource<T>(code, source, maxBufferLength());
    dispatchData->setValue(value);
  }

  template <typename T>
  void insertValues(DataCode code, const std::string& source,
                    const typename TimedSampleCollection<T>::TimedVector& values) {
    if (!values.empty()) {
      TypedDispatchData<T>* dispatchData =
        createDispatchDataForSource<T>(code, source, values.size());

      dispatchData->dispatcher()->insert(values);
    }
  }

  template<class T>
  void updateCurrentSource(DataCode code, TypedDispatchData<T>* dispatchData) {
    auto proxy = dynamic_cast<DispatchDataProxy<T>*>(this->dispatchData(code));
    assert(proxy != 0);

    if (!proxy->hasDispatcher() || prefers(dispatchData, proxy->realDispatcher())) {
      assert(dispatchData != nullptr);
      proxy->setActiveDispatcher(dispatchData);

      // Fire an event saying that 'code' is now provided by another source.
      dataSwitchedSource(dispatchData);
    }
  }

  bool prefers(DispatchData* a, DispatchData* b);
  int sourcePriority(const std::string& source) const;
  const std::map<std::string, int> &sourcePriority() const {
    return _sourcePriority;
  }
  void setSourcePriority(const std::string& source, int priority);

  boost::signals2::signal<void(DispatchData*)> newDispatchData;
  boost::signals2::signal<void(DispatchData*)> dataSwitchedSource;

  void set(DataCode code, const std::string &srcName,
      const std::shared_ptr<DispatchData> &d);

  template<DataCode Code>
  Optional<typename TypeForCode<Code>::type> valueFromSourceAt(
      const std::string& source, TimeStamp time,
      Duration<> maxDelta) const {
    typedef typename TypeForCode<Code>::type T;
    TypedDispatchData<T>* tdd =
      toTypedDispatchData<Code>(dispatchDataForSource(Code, source).get());
    if (tdd == nullptr) {
      return Optional<T>();
    }
    const TimedSampleCollection<T>& data = tdd->dispatcher()->values();
    auto nearest = findNearestTimedValue<T>(
        data.samples().begin(), data.samples().end(), time);
    if (nearest.defined()
        && fabs(nearest.get().time - time) < maxDelta) {
      return Optional<T>(nearest.get().value);
    } else {
      return Optional<T>();
    }
  }

  int maxPriority() const;

  std::vector<std::string> sourcesForChannel(DataCode code) const {
    std::vector<std::string> sources;
    auto it = _data.find(code);
    if ((it == _data.end()) || (it->second.size() == 0)) {
      return sources;
    }
    for (auto s : it->second) {
      sources.push_back(s.first);
    }
    return sources;
  }

  bool hasSource(DataCode code, const std::string& source) const {
    auto it = _data.find(code);
    return (it != _data.end()
            && it->second.find(source) != it->second.end());
  }
 protected:
  // Override if you want to use a particular DispatchData representation.
  // It must be downcastable to TypedDispatchData<T> with T
  // corresponding to 'code'.
  virtual DispatchData* createNewCustomDispatchData(
      DataCode code, const std::string& src, int size) {
    return nullptr;
  }
 private:
  template <typename T>
  TypedDispatchData<T>* createNewTypedDispatchData(
      DataCode code, const std::string& source, int size) {
    auto custom = createNewCustomDispatchData(code, source, size);
    if (bool(custom)) {
      auto typed = dynamic_cast<TypedDispatchData<T>*>(custom);
      assert(bool(typed));
      return typed;
    } else {
      return new TypedDispatchDataReal<T>(code, source, this, size);
    }
  }

  std::map<DataCode, std::map<std::string,
    std::shared_ptr<DispatchData>>> _data;

  static Dispatcher *_globalInstance;

  // _currentSource contains the proxies of different types.
  std::map<DataCode, std::shared_ptr<DispatchData>> _currentSource;

  std::map<std::string, int> _sourcePriority;
};

// A convenient visitor to subscribe to any dispatch data type.
template <class Listener>
class SubscribeVisitor : public DispatchDataVisitor {
 public:
  SubscribeVisitor(Listener *listener) : listener_(listener) { }

  virtual void run(DispatchAngleData *angle) {
    angle->dispatcher()->subscribe(listener_);
  }

  virtual void run(DispatchVelocityData *velocity) {
    velocity->dispatcher()->subscribe(listener_);
  }

  virtual void run(DispatchLengthData *data) {
    data->dispatcher()->subscribe(listener_);
  }

  virtual void run(DispatchGeoPosData *data) {
    data->dispatcher()->subscribe(listener_);
  }

  virtual void run(DispatchTimeStampData *data) {
    data->dispatcher()->subscribe(listener_);
  }

  virtual void run(DispatchAbsoluteOrientationData *data) {
    data->dispatcher()->subscribe(listener_);
  }

  virtual void run(DispatchBinaryEdge *data) {
    data->dispatcher()->subscribe(listener_);
  }

  virtual void run(DispatchAngularVelocityData *data) {
    data->dispatcher()->subscribe(listener_);
  }

 private:
  Listener *listener_;
};


template <typename T>
TypedDispatchData<T>* Dispatcher::createDispatchDataForSource(
    DataCode code, const std::string& source, int size) {
  auto ptr = dispatchDataForSource(code, source);

  TypedDispatchData<T>* dispatchData;
  if (!ptr) {
    dispatchData = createNewTypedDispatchData<T>(code, source, size);
    _data[code][source] = std::shared_ptr<DispatchData>(dispatchData);
    newDispatchData(dispatchData);
  } else {
    dispatchData = dynamic_cast<TypedDispatchData<T>*>(ptr.get());
    // wrong type for this code.
    assert(dispatchData);
  }

  updateCurrentSource(code, dispatchData);
  return dispatchData;
}

int getSourcePriority(const std::map<std::string, int> &sourcePriority, const std::string &source);


}  // namespace sail

#endif  // ANEMOBOX_DISPATCHER_H
