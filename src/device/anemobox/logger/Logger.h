#ifndef ANEMOBOX_LOGGER_H
#define ANEMOBOX_LOGGER_H

#include <cstdint>
#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/logger/logger.pb.h>
#include <boost/signals2/connection.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace sail {

class Logger;

void addTimeStampToRepeatedFields(
    std::int64_t *base,
    google::protobuf::RepeatedField<std::int64_t> *dst,
    TimeStamp);

Optional<int64_t> readIntegerFromTextFile(const std::string &filename);

void unpackTimeStamps(
    const google::protobuf::RepeatedField<std::int64_t> &times,
    std::vector<TimeStamp>* result);


// Listen and save a single stream of values.
class LoggerValueListener:
  public Listener<Angle<double>>,
  public Listener<Velocity<double>>,
  public Listener<Length<double>>,
  public Listener<GeographicPosition<double>>,
  public Listener<TimeStamp>,
  public Listener<AbsoluteOrientation>,
  public Listener<BinaryEdge> {
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
    addTimeStampToRepeatedFields(&timestampBase,
        _valueSet.mutable_timestampssinceboot(), timestamp);
  }

  static void accumulateAngle(const Angle<> &angle, int *base, AngleValueSet* set) {
    int value = int(angle.degrees() * 100.0);
    int delta = value;
    if (set->deltaangle_size() > 0) {
      delta -= *base;
    }
    set->add_deltaangle(delta);
    *base = value;
  }

  virtual void onNewValue(const ValueDispatcher<Angle<double>> &angle) {
    addTimestamp(angle.lastTimeStamp());

    accumulateAngle(angle.lastValue(), &intBase, _valueSet.mutable_angles());
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

  virtual void onNewValue(const ValueDispatcher<TimeStamp> &v) {
    addTimestamp(v.lastTimeStamp());
    TimeStamp t = v.lastValue();
    addTimeStampToRepeatedFields(&extTimesBase, _valueSet.mutable_exttimes(), t);
  }

  virtual void onNewValue(const ValueDispatcher<AbsoluteOrientation> &v) {
    addTimestamp(v.lastTimeStamp());

    accumulateAngle(v.lastValue().heading, &intBase, 
                    _valueSet.mutable_orient()->mutable_heading());

    accumulateAngle(v.lastValue().roll, &intBaseRoll, 
                    _valueSet.mutable_orient()->mutable_roll());

    accumulateAngle(v.lastValue().pitch, &intBasePitch, 
                    _valueSet.mutable_orient()->mutable_pitch());
  }

  virtual void onNewValue(const ValueDispatcher<BinaryEdge> &v) {
    addTimestamp(v.lastTimeStamp());
    _valueSet.mutable_binary()->add_edges(v.lastValue() == BinaryEdge::ToOn);
  }

  void addText(TimeStamp t, const std::string& text) {
    addTimestamp(t);
    _valueSet.add_text(text);
  }

  const std::string& source() const { return _sourceName; }
private:
  ValueSet _valueSet;
  int intBase = 0;
  int intBaseRoll = 0;
  int intBasePitch = 0;
  std::int64_t timestampBase = 0;
  std::int64_t extTimesBase = 0;
  std::string _sourceName;
  std::string _shortName;
};

enum class Nmea2000SizeClass {Regular, Odd};

inline Nmea2000SizeClass getNmea2000SizeClass(size_t byteCount) {
  return byteCount == 8?
      Nmea2000SizeClass::Regular
      : Nmea2000SizeClass::Odd;
}

class Nmea2000SentenceAccumulator {
public:
  void add(const TimeStamp& time,
	   int64_t id, size_t count, const char* data);
  Nmea2000Sentences* mutableData() {return &_data;}
private:
  Nmea2000SizeClass _sizeClass = Nmea2000SizeClass::Regular;
  std::int64_t timestampBase = 0;
  Nmea2000Sentences _data;
};

class Logger {
 public:
  Logger(Dispatcher* dispatcher);

  // Move all stored data into <container>. This method should
  // be called in the dispatcher thread.
  void flushTo(LogFile* container);

  // Convenience function to call flushTo, nextFilename and save.
  bool flushAndSaveToFile(const std::string& filename);

  void logText(const std::string& streamName, 
	       const std::string& content);

  void logRawNmea2000(
        int64_t timestampMillisecondsSinceBoot,
        int64_t id,
        size_t count, const char* data);

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

  static void unpack(const AbsOrientValueSet& values,
                     std::vector<AbsoluteOrientation>* result);

  static void unpack(const BinaryEdgeValueSet& values,
                     std::vector<BinaryEdge>* result);

  static void unpack(const google::protobuf::RepeatedField<std::int64_t> &times,
                      std::vector<TimeStamp>* result);

  static void unpackTime(const ValueSet& valueSet,
                         std::vector<TimeStamp>* result);

  static void unpackTime(const Nmea2000Sentences& sentences,
                         std::vector<TimeStamp>* result);

  // For unit testing
  int numListeners() const { return _listeners.size(); }

 private:
  void subscribe();
  void subscribeToDispatcher(DispatchData *d);

  Dispatcher* _dispatcher;
  std::vector<std::shared_ptr<LoggerValueListener>> _listeners;
  std::map<std::string, LoggerValueListener> _textLoggers;
  std::map<std::pair<int64_t, Nmea2000SizeClass>, Nmea2000SentenceAccumulator> _rawNmea2000Sentences;
  boost::signals2::scoped_connection _newDispatchDataListener;
};

template <typename T>
struct ValueSetToTypedVector {
};

template <>
struct ValueSetToTypedVector<Angle<double> > {
  static void extract(const ValueSet &x, std::vector<Angle<double> > *dst) {
    Logger::unpack(x.angles(), dst);
  }
};

template <>
struct ValueSetToTypedVector<Velocity<double> > {
  static void extract(const ValueSet &x, std::vector<Velocity<double> > *dst) {
    Logger::unpack(x.velocity(), dst);
  }
};

template <>
struct ValueSetToTypedVector<Length<double> > {
  static void extract(const ValueSet &x, std::vector<Length<double> > *dst) {
    Logger::unpack(x.length(), dst);
  }
};

template <>
struct ValueSetToTypedVector<GeographicPosition<double> > {
  static void extract(const ValueSet &x, std::vector<GeographicPosition<double> > *dst) {
    Logger::unpack(x.pos(), dst);
  }
};

template <>
struct ValueSetToTypedVector<AbsoluteOrientation> {
  static void extract(const ValueSet &x, std::vector<AbsoluteOrientation> *dst) {
    Logger::unpack(x.orient(), dst);
  }
};

template <>
struct ValueSetToTypedVector<TimeStamp> {
  static void extract(const ValueSet &x, std::vector<TimeStamp> *dst) {
    Logger::unpackTime(x, dst);
  }
};

template <>
struct ValueSetToTypedVector<BinaryEdge> {
  static void extract(const ValueSet &x, std::vector<BinaryEdge> *dst) {
    Logger::unpack(x.binary(), dst);
  }
};
}  // namespace sail

#endif   // ANEMOBOX_LOGGER_H
