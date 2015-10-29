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

// For compatibility: never re-use or change a number.
// When adding fields, keep increasing.
// When removing a field, never recycle its index value.
enum DataCode {
  AWA = 1,
  AWS = 2,
  TWA = 3,
  TWS = 4,
  TWDIR = 5,
  GPS_SPEED = 6,
  GPS_BEARING = 7,
  MAG_HEADING = 8,
  WAT_SPEED = 9,
  WAT_DIST = 10,
  GPS_POS = 11,
  DATE_TIME = 12,
  TARGET_VMG = 13,
  VMG = 14,
  ORIENT = 15,
  NUM_DATA_CODE = 16
};

template <DataCode Code> struct TypeForCode { };

template<> struct TypeForCode<AWA> { typedef Angle<> type; };
template<> struct TypeForCode<AWS> { typedef Velocity<> type; };
template<> struct TypeForCode<TWA> { typedef Angle<> type; };
template<> struct TypeForCode<TWS> { typedef Velocity<> type; };
template<> struct TypeForCode<TWDIR> { typedef Angle<> type; };
template<> struct TypeForCode<GPS_SPEED> { typedef Velocity<> type; };
template<> struct TypeForCode<GPS_BEARING> { typedef Angle<> type; };
template<> struct TypeForCode<MAG_HEADING> { typedef Angle<> type; }; 
template<> struct TypeForCode<WAT_SPEED> { typedef Velocity<> type; };
template<> struct TypeForCode<WAT_DIST> { typedef Length<> type; };
template<> struct TypeForCode<GPS_POS> { typedef GeographicPosition<double> type; };
template<> struct TypeForCode<DATE_TIME> { typedef TimeStamp type; };
template<> struct TypeForCode<TARGET_VMG> { typedef Velocity<> type; };
template<> struct TypeForCode<VMG> { typedef Velocity<> type; };
template<> struct TypeForCode<ORIENT> { typedef AbsoluteOrientation type; };

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

  DispatchData *dispatchDataForSource(DataCode code, const std::string& source);

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

  template <DataCode Code>
  typename TypeForCode<Code>::type val() {
    return get<Code>()->dispatcher()->lastValue();
  }

  int onNewSource(std::function<void(DataCode, const std::string&)> f);
  void removeNewSourceListener(int);

  template <typename T>
    void publishValue(DataCode code, const std::string& source, T value) {
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
      dispatchData->setValue(value);
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
  template <DataCode Code> void registerCode();

  static Dispatcher *_globalInstance;

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


}  // namespace sail

#endif  // ANEMOBOX_DISPATCHER_H
