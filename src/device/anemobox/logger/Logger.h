#ifndef ANEMOBOX_LOGGER_H
#define ANEMOBOX_LOGGER_H

#include <device/anemobox/Dispatcher.h>
#include <logger.pb.h>

#include <boost/signals2/connection.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace sail {

class Logger;

// Listen and save a single stream of values.
class LoggerValueListener:
  public Listener<Angle<double>>,
  public Listener<Velocity<double>>,
  public Listener<Length<double>>,
  public Listener<GeographicPosition<double>>,
  public Listener<TimeStamp> {
public:
  LoggerValueListener(const std::string& shortName,
                      const std::string& sourceName)
    : _sourceName(sourceName), _shortName(shortName) {
    clear();
  }
  LoggerValueListener(const LoggerValueListener& other) = default;

  const ValueSet& valueSet() const { return _valueSet; }
  ValueSet* mutable_valueSet() { return &_valueSet; }

  void clear() {
    _valueSet = ValueSet();
    _valueSet.set_shortname(_shortName);
    _valueSet.set_source(_sourceName);
  }

  void setPriority(Dispatcher* dispatcher) {
    int priority = dispatcher->sourcePriority(_sourceName);
    if (priority != 0) {
      _valueSet.set_priority(priority);
    }
  }

  void addTimestamp(const TimeStamp& timestamp) {
    int64_t ts = timestamp.toMilliSecondsSince1970();
    int64_t value = ts;

    if (_valueSet.timestamps_size() > 0) {
      value -= timestampBase;
    }
    timestampBase = ts;
    _valueSet.add_timestamps(value);
  }

  virtual void onNewValue(const ValueDispatcher<Angle<double>> &angle) {
    addTimestamp(angle.lastTimeStamp());

    int value = int(angle.lastValue().degrees() * 100.0);
    int delta = value;
    if (_valueSet.angles().deltaangle_size() > 0) {
      delta -= intBase;
    }
    _valueSet.mutable_angles()->add_deltaangle(delta);
    intBase = value;
  }


  virtual void onNewValue(const ValueDispatcher<Velocity<double>> &v) {
    addTimestamp(v.lastTimeStamp());

    int value = int(v.lastValue().knots() * 100.0);
    int delta = value;
    if (_valueSet.velocity().deltavelocity_size() > 0) {
      delta -= intBase;
    }
    _valueSet.mutable_velocity()->add_deltavelocity(delta);
    intBase = value;
  }

  virtual void onNewValue(const ValueDispatcher<Length<double>> &v) {
    addTimestamp(v.lastTimeStamp());

    int value = int(v.lastValue().meters());
    int delta = value;
    if (_valueSet.length().deltalength_size() > 0) {
      delta -= intBase;
    }
    _valueSet.mutable_length()->add_deltalength(delta);
    intBase = value;
  }

  virtual void onNewValue(const ValueDispatcher<GeographicPosition<double>> &v) {
    addTimestamp(v.lastTimeStamp());
    GeoPosValueSet_Pos* pos = _valueSet.mutable_pos()->add_pos();
    pos->set_lat(v.lastValue().lat().degrees());
    pos->set_lon(v.lastValue().lon().degrees());
  }

  virtual void onNewValue(const ValueDispatcher<TimeStamp> &) { }

  void addText(const std::string& text) {
    addTimestamp(TimeStamp::now());
    _valueSet.add_text(text);
  }

  const std::string& source() const { return _sourceName; }
private:
  ValueSet _valueSet;
  int intBase;
  int64_t timestampBase;
  std::string _sourceName;
  std::string _shortName;
};

class Logger {
 public:
  Logger(Dispatcher* dispatcher);

  // Move all stored data into <container>. This method should
  // be called in the dispatcher thread. 
  void flushTo(LogFile* container);

  // Convenience function to call flushTo, nextFilename and save.
  bool flushAndSave(const std::string& folder, std::string *savedFilename = 0);

  // Generate a new filename to save the next logfile to.
  static std::string nextFilename(const std::string& folder);

  void logText(const std::string& streamName, const std::string& content);

  // Save invokes gzip, it might be slightly time consuming.
  static bool save(const std::string& filename, const LogFile& data);
  static bool read(const std::string& filename, LogFile *dst);

  static void unpack(const AngleValueSet& values,
                     std::vector<Angle<double>>* angles);

  static void unpack(const VelocityValueSet& values,
                     std::vector<Velocity<double>>* result);

  static void unpack(const LengthValueSet& values,
                     std::vector<Length<double>>* result);

  static void unpack(const GeoPosValueSet& values,
                     std::vector<GeographicPosition<double>>* result);

  static void unpackTime(const ValueSet& valueSet,
                         std::vector<TimeStamp>* result);

  // For unit testing
  int numListeners() const { return _listeners.size(); }

 private:
  void subscribe();
  void subscribeToDispatcher(DispatchData *d);

  Dispatcher* _dispatcher;
  std::vector<std::shared_ptr<LoggerValueListener>> _listeners;
  std::map<std::string, LoggerValueListener> _textLoggers;
  boost::signals2::scoped_connection _newDispatchDataListener;
};

}  // namespace sail

#endif   // ANEMOBOX_LOGGER_H
