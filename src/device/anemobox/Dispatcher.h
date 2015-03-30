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
enum DataMeaning {
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

class DataSource {
 public:
  virtual const char* name() const = 0;
};

class DispatchData {
 public:
  DispatchData(std::map<DataMeaning, DispatchData*> *index,
               DataMeaning nature,
               std::string description)
    : _nature(nature), _description(description) { (*index)[nature] = this; }

  std::string description() const { return _description; }
  DataMeaning nature() const { return _nature; }
  virtual std::string valueAsString() const = 0;
  virtual std::string unitAsString() const = 0;
 private:
  DataMeaning _nature;
  std::string _description;
};

class DispatchAngleData : public DispatchData {
 public:
   DispatchAngleData(std::map<DataMeaning, DispatchData*> *index,
                     DataMeaning nature,
                     std::string description)
     : DispatchData(index, nature, description), _dispatcher(1024) { }

  virtual std::string valueAsString() const {
    return stringFormat("%.1f", _dispatcher.lastValue().degrees());
  }
  virtual std::string unitAsString() const { return "degrees"; };

  AngleDispatcher *dispatcher() { return &_dispatcher; }
 private:
  AngleDispatcher _dispatcher;
};

class DispatchVelocityData : public DispatchData {
 public:
   DispatchVelocityData(std::map<DataMeaning, DispatchData*> *index,
                        DataMeaning nature,
                        std::string description)
     : DispatchData(index, nature, description), _dispatcher(1024) { }

  virtual std::string valueAsString() const {
    return stringFormat("%.2f", _dispatcher.lastValue().knots());
  }
  virtual std::string unitAsString() const { return "knots"; };

  VelocityDispatcher *dispatcher() { return &_dispatcher; }
 private:
  VelocityDispatcher _dispatcher;
};

// Dispatcher: the hub for all values processed by the anemobox.
// the data() method allows enumeration of all components.
class Dispatcher {
 public:
  Dispatcher();

  const DispatchData& dispatchData(DataMeaning index) const; 
  const std::map<DataMeaning, DispatchData*> data() const { return _data; }

  AngleDispatcher* awa() { return _awa.dispatcher(); }
  VelocityDispatcher* aws() { return _aws.dispatcher(); }
  AngleDispatcher* twa() { return _twa.dispatcher(); }
  VelocityDispatcher* tws() { return _tws.dispatcher(); }
  AngleDispatcher* twdir() { return _twdir.dispatcher(); }
  AngleDispatcher* gpsBearing() { return _gpsBearing.dispatcher(); }
  VelocityDispatcher* gpsSpeed() { return _gpsSpeed.dispatcher(); }
  AngleDispatcher* magHeading() { return _magHeading.dispatcher(); }
  VelocityDispatcher* watSpeed() { return _watSpeed.dispatcher(); }

  template <class T>
  void publishValue(ValueDispatcher<T> *dispatcher, DataSource *source, T value) {
    // TODO: check if <source> is the current preferred source for this dispatcher.
    dispatcher->setValue(value);
  }

 private:
  void add(DispatchData *d);
  std::map<DataMeaning, DispatchData*> _data;

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
