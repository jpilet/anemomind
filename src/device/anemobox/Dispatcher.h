#ifndef ANEMOBOX_DISPATCHER_H
#define ANEMOBOX_DISPATCHER_H

// The main class of this file is 'Dispatcher'.
// The dispatcher receive data from one or more DataSource, and publish it
// to consumers that can subscribe to any value.

#include <string>
#include <map>

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
  WAT_SPEED = 9
};

class DispatchDataVisitor;
class DispatchData {
 public:
  DispatchData(std::map<DataCode, DispatchData*> *index,
               DataCode code,
               std::string wordIdentifier,
               std::string description)
    : _code(code),
    _description(description),
    _wordIdentifier(wordIdentifier) { (*index)[code] = this; }

  std::string description() const { return _description; }
  DataCode dataCode() const { return _code; }


  //! returns a single word that describe what this value is.
  // For example: awa, tws, watSpeed, etc.
  std::string wordIdentifier() const { return _wordIdentifier; }

  virtual void visit(DispatchDataVisitor *visitor) = 0;
 private:
  DataCode _code;
  std::string _description;
  std::string _wordIdentifier;
};

template <typename T>
class TypedDispatchData : public DispatchData {
 public:
  TypedDispatchData(std::map<DataCode, DispatchData*> *index,
                    DataCode nature,
                    std::string wordIdentifier,
                    std::string description)
     : DispatchData(index, nature, wordIdentifier, description),
     _dispatcher(1024) { }

  virtual void visit(DispatchDataVisitor *visitor);
  ValueDispatcher<T> *dispatcher() { return &_dispatcher; }
  const ValueDispatcher<T> *dispatcher() const { return &_dispatcher; }

  void publishValue(const char *source, T value) {
    // TODO: check if <source> is the current preferred source for this dispatcher.
    _dispatcher.setValue(value);
  }

 private:
  ValueDispatcher<T> _dispatcher;
};
typedef TypedDispatchData<Angle<double>> DispatchAngleData;
typedef TypedDispatchData<Velocity<double>> DispatchVelocityData;

class DispatchDataVisitor {
 public:
  virtual void run(DispatchAngleData *angle) = 0;
  virtual void run(DispatchVelocityData *velocity) = 0;
};

template <typename T>
void TypedDispatchData<T>::visit(DispatchDataVisitor *visitor) {
  visitor->run(this);
}

//! Dispatcher: the hub for all values processed by the anemobox.
// the data() method allows enumeration of all components.
class Dispatcher {
 public:
  Dispatcher();

  //! Get a pointer to the default anemobox dispatcher.
  static Dispatcher *global();

  const DispatchData& dispatchData(DataCode index) const; 
  const std::map<DataCode, DispatchData*> data() const { return _data; }

  DispatchAngleData* awa() { return &_awa; }
  DispatchVelocityData* aws() { return &_aws; }
  DispatchAngleData* twa() { return &_twa; }
  DispatchVelocityData* tws() { return &_tws; }
  DispatchAngleData* twdir() { return &_twdir; }
  DispatchAngleData* gpsBearing() { return &_gpsBearing; }
  DispatchVelocityData* gpsSpeed() { return &_gpsSpeed; }
  DispatchAngleData* magHeading() { return &_magHeading; }
  DispatchVelocityData* watSpeed() { return &_watSpeed; }

 private:
  static Dispatcher *_globalInstance;
  std::map<DataCode, DispatchData*> _data;

  DispatchAngleData _awa;
  DispatchVelocityData _aws;
  DispatchAngleData _twa;
  DispatchVelocityData _tws;
  DispatchAngleData _twdir;
  DispatchAngleData _gpsBearing;
  DispatchVelocityData _gpsSpeed;
  DispatchAngleData _magHeading;
  DispatchVelocityData _watSpeed;
};

}  // namespace sail

#endif  // ANEMOBOX_DISPATCHER_H
