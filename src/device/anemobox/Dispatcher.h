#ifndef ANEMOBOX_DISPATCHER_H
#define ANEMOBOX_DISPATCHER_H

// The main class of this file is 'Dispatcher'.
// The dispatcher receive data from one or more DataSource, and publish it
// to consumers that can subscribe to any value.

#include <boost/signals2/signal.hpp>
#include <map>
#include <memory>
#include <string>

#include <device/anemobox/ValueDispatcher.h>
#include <server/common/string.h>

namespace sail {

/*
 List of channels used by Dispatcher and NavHistory.

 This macro provides an easy way to iterate over channels at compile time.

 To use this macro, define a macro that takes the following arguments:
  #define ENUM_ENTRY(handle, code, type, shortname, description)
 
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
  X(ORIENT, 15, "orient", AbsoluteOrientation, "Absolute anemobox orientation")

enum DataCode {
#define ENUM_ENTRY(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  HANDLE = CODE,

  FOREACH_CHANNEL(ENUM_ENTRY)
#undef ENUM_ENTRY
};

template <DataCode Code> struct TypeForCode { };

#define DECL_TYPE(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
template<> struct TypeForCode<HANDLE> { typedef TYPE type; };
FOREACH_CHANNEL(DECL_TYPE)
#undef DECL_TYPE

const char* descriptionForCode(DataCode code);
const char* wordIdentifierForCode(DataCode code);

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

template <typename T>
class TypedDispatchDataReal : public TypedDispatchData<T> {
 public:
  TypedDispatchDataReal(DataCode nature,
                        std::string source,
                        Clock* clock, int bufferLength=1024)
     : TypedDispatchData<T>(nature, source),
     _dispatcher(clock, bufferLength) { }

  virtual ValueDispatcher<T> *dispatcher() { return &_dispatcher; }
  virtual const ValueDispatcher<T> *dispatcher() const { return &_dispatcher; }

  virtual void setValue(T value) { _dispatcher.setValue(value); }

 private:
  ValueDispatcher<T> _dispatcher;
};
typedef TypedDispatchData<Angle<double>> DispatchAngleData;
typedef TypedDispatchData<Velocity<double>> DispatchVelocityData;
typedef TypedDispatchData<Length<double>> DispatchLengthData;
typedef TypedDispatchData<GeographicPosition<double>> DispatchGeoPosData;
typedef TypedDispatchData<TimeStamp> DispatchTimeStampData;
typedef TypedDispatchData<AbsoluteOrientation> DispatchAbsoluteOrientationData;

template <typename T>
class DispatchDataProxy : public TypedDispatchData<T> {
 public:
   DispatchDataProxy(DataCode code) : TypedDispatchData<T>(code, "") { }

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
  virtual ~DispatchDataVisitor() {}
};

template <typename T>
void TypedDispatchData<T>::visit(DispatchDataVisitor *visitor) {
  visitor->run(this);
}

//! Dispatcher: the hub for all values processed by the anemobox.
// the data() method allows enumeration of all components.
class Dispatcher : public Clock {
 public:
  Dispatcher();

  //! Get a pointer to the default anemobox dispatcher.
  static Dispatcher *global();

  const std::map<DataCode, std::shared_ptr<DispatchData>>& dispatchers()
    const { return _currentSource; }

  const std::map<DataCode, std::map<std::string, DispatchData*>> allSources() {
    return _data;
  }

  // Returns null if not exist
  DispatchData *dispatchDataForSource(DataCode code, const std::string& source);

  // Return or create a DispatchData for the given source.
  template <typename T>
  TypedDispatchData<T>* createDispatchDataForSource(
      DataCode code, const std::string& source);

  DispatchData* dispatchData(DataCode code) const {
    auto it = _currentSource.find(code);
    assert (it != _currentSource.end());
    return it->second.get();
  }

  template <DataCode Code>
  TypedDispatchData<typename TypeForCode<Code>::type>* get() const {
    return dynamic_cast<TypedDispatchData<typename TypeForCode<Code>::type>*>(
        dispatchData(Code));
  }

  // When the Dispatcher is used to hold a loaded dataset,
  // use this method to access the samples.
  template <DataCode Code>
  const TimedSampleCollection<typename TypeForCode<Code>::type>& getSamples() const {
    return get<Code>()->dispatcher()->values();
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
      createDispatchDataForSource<T>(code, source);

    dispatchData->setValue(value);
  }

  template <typename T>
  void insertValues(DataCode code, const std::string& source,
                    const typename TimedSampleCollection<T>::TimedVector& values) {
    TypedDispatchData<T>* dispatchData =
      createDispatchDataForSource<T>(code, source);

    dispatchData->dispatcher()->insert(values);
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
  int sourcePriority(const std::string& source);
  void setSourcePriority(const std::string& source, int priority);

  boost::signals2::signal<void(DispatchData*)> newDispatchData;
  boost::signals2::signal<void(DispatchData*)> dataSwitchedSource;

 private:
  static Dispatcher *_globalInstance;

  // TODO: Do we delete the points to DispatchData anywhere?
  // Should we wrap them inside std::shared_ptr?
  std::map<DataCode, std::map<std::string, DispatchData*>> _data;

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

 private:
  Listener *listener_;
};


template <typename T>
TypedDispatchData<T>* Dispatcher::createDispatchDataForSource(
    DataCode code, const std::string& source) {
  auto ptr = dispatchDataForSource(code, source);

  TypedDispatchData<T>* dispatchData;
  if (!ptr) {
    _data[code][source] = dispatchData = 
      new TypedDispatchDataReal<T>(code, source, this);
    newDispatchData(dispatchData);
  } else {
    dispatchData = dynamic_cast<TypedDispatchData<T>*>(ptr);
    // wrong type for this code.
    assert(dispatchData);
  }

  updateCurrentSource(code, dispatchData);
  return dispatchData;
}


}  // namespace sail

#endif  // ANEMOBOX_DISPATCHER_H
