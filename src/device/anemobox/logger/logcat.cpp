#include <device/anemobox/logger/Logger.h>
#include <logger.pb.h>
#include <server/common/logging.h>

#include <iostream>
#include <iomanip>

using namespace sail;
using namespace std;

namespace {

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
                  const vector<T>& values) {
  if (times.size() != values.size()) {
    LOG(WARNING) << "time and value array do not have the same size!";
  }
  const int len = min(times.size(), values.size());
  for (int i = 0; i < len; ++i) {
    cout << "  " << times[i].toString() << " " << values[i] << endl;
  }
}
  
void streamCat(const ValueSet& valueSet) {
  cout << "\n" << valueSet.shortname() << ":\n";
  vector<TimeStamp> times;
  Logger::unpackTime(valueSet, &times);

  if (valueSet.has_angles()) {
    vector<Angle<double>> angles;
    Logger::unpack(valueSet.angles(), &angles);
    formatValues(times, angles);
  }
  if (valueSet.has_velocity()) {
    vector<Velocity<double>> values;
    Logger::unpack(valueSet.velocity(), &values);
    formatValues(times, values);
  }
  if (valueSet.has_length()) {
    vector<Length<double>> values;
    Logger::unpack(valueSet.length(), &values);
    formatValues(times, values);
  }
  if (valueSet.has_pos()) {
    vector<GeographicPosition<double>> values;
    Logger::unpack(valueSet.pos(), &values);
    formatValues(times, values);
  }

  for (int i = 0; i < valueSet.text_size(); ++i) {
    cout << "  " << times[i].toString() << ": \""
      << valueSet.text(i) << "\"\n";
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

  for (int i = 0; i < data.stream_size(); ++i) {
    streamCat(data.stream(i));
  }

  for (int i = 0; i < data.text_size(); ++i) {
    streamCat(data.text(i));
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
}

