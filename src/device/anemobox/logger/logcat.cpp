#include <device/anemobox/logger/Logger.h>
#include <logger.pb.h>
#include <server/common/logging.h>

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

using namespace sail;
using namespace std;

namespace {

struct TimedString {
  TimeStamp time;
  std::string str;

  TimedString(TimeStamp t, const string& s) : time(t), str(s) { }

  bool operator < (const TimedString& other) const {
    return time < other.time;
  }
};

ostream& operator<<(ostream& out, const Angle<double>& angle) {
  return out << angle.degrees() << " deg.";
}
ostream& operator<<(ostream& out, const Velocity<double>& value) {
  return out << value.knots() << " kn.";
}
ostream& operator<<(ostream& out, const Length<double>& value) {
  return out << value.nauticalMiles() << " miles.";
}
ostream& operator<<(ostream& out, const GeographicPosition<double>& value) {
  // this format is compatible with google maps.
  return out << setprecision(10) << "(" << value.lat().degrees() << ", "
    << setprecision(10)<< value.lon().degrees() << ")";
}

template <class T>
void formatValues(const vector<TimeStamp>& times,
                  const vector<T>& values,
                  const string& name,
                  vector<TimedString>* result) {
  if (times.size() != values.size()) {
    LOG(WARNING) << "time and value array do not have the same size!";
  }
  const int len = min(times.size(), values.size());
  for (int i = 0; i < len; ++i) {
    ostringstream s;
    s << name << ": " << values[i];
    result->push_back(TimedString(times[i], s.str()));
  }
}
  
void streamCat(const ValueSet& valueSet, vector<TimedString>* entries) {
  vector<TimeStamp> times;
  Logger::unpackTime(valueSet, &times);

  ostringstream s;
  s << valueSet.shortname() << "[" << valueSet.source() << ":" 
    << (valueSet.has_priority() ? valueSet.priority() : 0) << "]";
  std::string prefix = s.str();

  if (valueSet.has_angles()) {
    vector<Angle<double>> angles;
    Logger::unpack(valueSet.angles(), &angles);
    formatValues(times, angles, prefix, entries);
  }
  if (valueSet.has_velocity()) {
    vector<Velocity<double>> values;
    Logger::unpack(valueSet.velocity(), &values);
    formatValues(times, values, prefix, entries);
  }
  if (valueSet.has_length()) {
    vector<Length<double>> values;
    Logger::unpack(valueSet.length(), &values);
    formatValues(times, values, prefix, entries);
  }
  if (valueSet.has_pos()) {
    vector<GeographicPosition<double>> values;
    Logger::unpack(valueSet.pos(), &values);
    formatValues(times, values, prefix, entries);
  }

  for (int i = 0; i < valueSet.text_size(); ++i) {
    entries->push_back(TimedString(times[i], prefix + ": " + valueSet.text(i)));
  }
}

void logCat(const char* file) {
  LogFile data;
  if (!Logger::read(file, &data)) {
    LOG(ERROR) << file << ": can't read log file.";
    return;
  }

  if (data.has_anemobox()) {
    cout << "Anemobox: " << data.anemobox() << endl;
  }
  if (data.has_boatid()) {
    cout << "boatId: " << data.boatid() << endl;
  }

  if (data.has_boatname()) {
    cout << "boatName: " << data.boatname() << endl;
  }

  vector<TimedString> entries;
  for (int i = 0; i < data.stream_size(); ++i) {
    streamCat(data.stream(i), &entries);
  }

  for (int i = 0; i < data.text_size(); ++i) {
    streamCat(data.text(i), &entries);
  }

  sort(entries.begin(), entries.end());

  for (const TimedString& entry : entries) {
    cout << entry.time.toString() << ": " << entry.str << endl;
  }
}

}  // namespace

int main(int argc, const char* argv[]) {

  if (argc < 1) {
    LOG(FATAL) << "Usage: " << argv[0] << " <logfile> [<logfile> ...]";
  }

  for (int i = 1; i < argc; ++i) {
    logCat(argv[i]);
  }
  return 0;
}

