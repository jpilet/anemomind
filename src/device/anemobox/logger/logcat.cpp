/*
 * To get the internal GPS NMEA stream, use:
 * ./anemobox_logcat -t "Internal GPS NMEA" <logfile>
 */

#include <device/anemobox/logger/Logger.h>
#include <device/anemobox/logger/logger.pb.h>
#include <server/common/ArgMap.h>
#include <server/common/logging.h>
#include <server/common/Span.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

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

struct SubRange {
  Span<TimeStamp> span;
  int count = 0;
  SubRange() {}
  SubRange(const Span<TimeStamp> &ts, int n) : span(ts), count(n) {}
};

class ChannelSummary {
public:
  ChannelSummary() {}

  ChannelSummary(
      const std::string &code,
      const std::string &s,
      const std::vector<SubRange> &subRanges) :
        code(code), src(s), subRanges(subRanges) {}

  bool defined() const {return !src.empty();}

  static ChannelSummary make(
      const std::string &code, const std::string &src,
      const std::vector<TimeStamp> times,
      const Duration<double> &threshold);

  std::string code;
  std::string src;
  std::vector<SubRange> subRanges;
};

std::ostream &operator<<(std::ostream &s, const ChannelSummary &cs) {
  s << "\n  * code='" << cs.code
      << "' src='" << cs.src
      << (cs.subRanges.empty()? "'" : "':");
  for (auto sr: cs.subRanges) {
    s << "\n    - " << sr.count << " samples from " <<
        sr.span.minv() << " to " << sr.span.maxv();
  }
  return s;
}

typedef std::pair<std::string, std::string> Key;

class Summary {
public:
  Summary(Duration<double> d) : _threshold(d) {}
  void add(const std::string &c, const std::string &src,
      const std::vector<TimeStamp> &times);
  bool defined() const {return 0.0_s <= _threshold;}
  std::vector<ChannelSummary> getChannelSummaries() const;
private:
  std::map<Key, std::vector<TimeStamp>> _acc;
  Duration<double> _threshold;
};

std::ostream &operator<<(std::ostream &s, const Summary &c) {
  s << "\nSummary:";
  for (auto x: c.getChannelSummaries()) {
    s << x;
  }
  return s;
}

void Summary::add(const std::string &c, const std::string &src,
    const std::vector<TimeStamp> &times) {
  auto &dst = _acc[Key(c, src)];
  dst.insert(dst.end(), times.begin(), times.end());
}

std::vector<ChannelSummary> Summary::getChannelSummaries() const {
  std::vector<ChannelSummary> dst;
  for (auto kv: _acc) {
    auto key = kv.first;
    auto times = kv.second;
    std::sort(times.begin(), times.end());
    dst.push_back(ChannelSummary::make(
        key.first, key.second, times, _threshold));
  }
  return dst;
}

std::vector<int> getBounds(
    const std::vector<TimeStamp> &times,
    Duration<double> threshold) {
  std::vector<int> gaps;
  gaps.push_back(0);
  for (int i = 1; i < times.size(); i++) {
    if (times[i] - times[i-1] > threshold) {
      gaps.push_back(i);
    }
  }
  gaps.push_back(times.size());
  return gaps;
}

std::vector<SubRange> makeRanges(
    const std::vector<TimeStamp> &times,
    const std::vector<int> &bds) {
  int n = bds.size() - 1;
  std::vector<SubRange> rng;
  rng.reserve(n);
  for (int i = 0; i < n; i++) {
    auto from = bds[i];
    auto to = bds[i+1];
    CHECK(from < to);
    rng.push_back(SubRange{Span<TimeStamp>(
        times[from], times[to-1]), to - from});
  }
  return rng;
}

ChannelSummary ChannelSummary::make(
    const std::string &c, const std::string &src,
    const std::vector<TimeStamp> times,
    const Duration<double> &threshold) {
  if (times.empty()) {
    return ChannelSummary(c, src, {});
  }
  auto bds = getBounds(times, threshold);
  return ChannelSummary(c, src, makeRanges(times, bds));
}

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
ostream& operator<<(ostream& out, const AbsoluteOrientation& value) {
  return out
    << setprecision(2) << fixed << "heading: " << value.heading.degrees()
    << setprecision(2) << fixed << " roll: " << value.roll.degrees()
    << setprecision(2) << fixed << " pitch: " << value.pitch.degrees();
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

template <>
void formatValues<TimeStamp>(const vector<TimeStamp>& times,
                             const vector<TimeStamp>& values,
                             const string& name,
                             vector<TimedString>* result) {
  if (times.size() != values.size()) {
    LOG(WARNING) << "time and value array do not have the same size!";
  }
  const int len = min(times.size(), values.size());
  for (int i = 0; i < len; ++i) {
    ostringstream s;
    s << name << ": " << values[i].fullPrecisionString();
    result->push_back(TimedString(times[i], s.str()));
  }
}

void summarizeValueSet(
    const ValueSet &valueSet,
    const std::vector<TimeStamp> &times,
    Summary *summary) {
  summary->add(
      valueSet.shortname(),
      valueSet.source(), times);
}

  
void streamCat(const ValueSet& valueSet,
    vector<TimedString>* entries,
    Summary* summary) {
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
  if (valueSet.has_orient()) {
    vector<AbsoluteOrientation> values;
    Logger::unpack(valueSet.orient(), &values);
    formatValues(times, values, prefix, entries);
  }
  if (valueSet.exttimes_size() > 0) {
    vector<TimeStamp> extTimes;
    Logger::unpack(valueSet.exttimes(), &extTimes);
    formatValues(times, extTimes, prefix, entries);
  }

  for (int i = 0; i < valueSet.text_size(); ++i) {
    entries->push_back(TimedString(times[i], prefix + ": " + valueSet.text(i)));
  }
  summarizeValueSet(valueSet, times, summary);
}

void dispEntries(std::vector<TimedString> *entries) {
  std::sort(entries->begin(), entries->end());
  for (const TimedString& entry : *entries) {
    cout << entry.time.fullPrecisionString() << ": " << entry.str << endl;
  }
}

void logCat(const LogFile& data, Summary *summary) {
  if (data.has_anemobox()) {
    cout << "Anemobox: " << data.anemobox() << endl;
  }
  if (data.has_boatid()) {
    cout << "boatId: " << data.boatid() << endl;
  }

  if (data.has_boatname()) {
    cout << "boatName: " << data.boatname() << endl;
  }

  if (data.has_bootcount()) {
    cout << "bootcount: " << data.bootcount() << endl;
  }

  vector<TimedString> entries;
  for (int i = 0; i < data.stream_size(); ++i) {
    streamCat(data.stream(i), &entries, summary);
  }

  for (int i = 0; i < data.text_size(); ++i) {
    streamCat(data.text(i), &entries, summary);
  }

  if (!summary->defined()) {
    dispEntries(&entries);
  }
}

void catField(const LogFile& data,
    const std::string& field,
    Summary *summary) {
  vector<TimedString> entries;
  for (int i = 0; i < data.text_size(); ++i) {
    auto valueSet = data.text(i);
    if (valueSet.shortname() == field) {
      vector<TimeStamp> times;
      Logger::unpackTime(valueSet, &times);
      for (int i = 0; i < valueSet.text_size(); ++i) {
        entries.push_back(TimedString(times[i], valueSet.text(i)));
      }
      summarizeValueSet(valueSet, times, summary);
    }
  }
  if (!summary->defined()) {
    dispEntries(&entries);
  }
}

void logCat(const std::string& file,
    const std::string& textField,
    Summary *summary) {

  LogFile data;
  if (!Logger::read(file, &data)) {
    LOG(ERROR) << file << ": can't read log file.";
    return;
  }

  if (textField == "") {
    logCat(data, summary);
  } else {
    catField(data, textField, summary);
  }
}

struct LexicalOrder {
  bool operator() (const ArgMap::Arg* a, const ArgMap::Arg* b) {
    return a->valueUntraced() < b->valueUntraced();
  }
};

}  // namespace

int main(int argc, const char* argv[]) {
  if (argc <= 1) {
    LOG(FATAL) << "Usage: " << argv[0] << " [-t <field>] <logfile> [<logfile> ...]";
  }

  ArgMap cmdLine;
  string textField;
  double summaryThreshold = -1;

  cmdLine.registerOption("-t", "output only the given text stream")
      .store(&textField)
      .setUnique();

  cmdLine.registerOption("-s", "Provide a summary, "
      "cutting the data by a threshold of given seconds")
      .store(&summaryThreshold)
      .setUnique();

  cmdLine.registerOption("-b", "A brief summary")
      .setUnique();

  if (cmdLine.parse(argc, argv) != ArgMap::Continue) {
    return -1;
  }

  if (cmdLine.optionProvided("-b")) {
    summaryThreshold = std::numeric_limits<double>::infinity();
  }

  Array<ArgMap::Arg*> files = cmdLine.freeArgs();
  sort(files.begin(), files.end(), LexicalOrder());

  Summary summary(summaryThreshold*1.0_s);
  for (auto arg : cmdLine.freeArgs()) {
    logCat(arg->value(), textField, &summary);
  }
  if (summary.defined()) {
    std::cout << summary << std::endl;
  }
  return 0;
}

