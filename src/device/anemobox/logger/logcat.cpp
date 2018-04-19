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

enum class DateFormat {
  HumanReadable,
  Nmea2000Replay
};

class Context {
public:
  Context(Duration<double> d) : _threshold(d) {}
  void addSummary(const std::string &c, const std::string &src,
      const std::vector<TimeStamp> &times);
  bool briefReport() const {return 0.0_s <= _threshold;}
  bool fullReport() const {return !briefReport();}
  std::vector<ChannelSummary> getChannelSummaries() const;

  // Settings
  DateFormat dateFormat = DateFormat::HumanReadable;
  bool withHeader = true;
  bool withText = true;
  bool withStreams = true;
  bool withNmea2000 = true;

private:
  std::map<Key, std::vector<TimeStamp>> _acc;

  /**
   * Threshold:
   *   If threshold is a non-negative number, it means that
   *   the user wants to see a summary at the end. Then
   *   the regular behaviour of logcat, of printing everything,
   *   is suppressed.
   *
   *   Otherwise, if it is a negative number, no summary will be
   *   produced, but instead all data will be displayed.
   *
   */
  Duration<double> _threshold;
};

std::ostream &operator<<(std::ostream &s, const Context &c) {
  s << "\nSummary:";
  for (auto x: c.getChannelSummaries()) {
    s << x;
  }
  return s;
}

void Context::addSummary(const std::string &c, const std::string &src,
    const std::vector<TimeStamp> &times) {
  auto &dst = _acc[Key(c, src)];
  dst.insert(dst.end(), times.begin(), times.end());
}

std::vector<ChannelSummary> Context::getChannelSummaries() const {
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
    Context *summary) {
  summary->addSummary(
      valueSet.shortname(),
      valueSet.source(), times);
}

  
void streamCat(const ValueSet& valueSet,
    vector<TimedString>* entries,
    Context* summary) {
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

std::string formatNmea2000Data(
    int n, const uint8_t* data, bool swap) {
  std::stringstream ss;
  for (int i = 0; i < n; i++) {
    auto x = data[swap ? n - i - 1 : i];
    ss << std::setfill('0')
       << std::setw(2) << std::hex
       << std::uppercase << x;
  }
  auto s = ss.str();
  CHECK(s.length() == 2*n);
  return s;
}

std::string formatNmea2000Data(
    const std::string& data, bool swap) {
  return formatNmea2000Data(
      data.length(),
      reinterpret_cast<const uint8_t*>(data.c_str()),
      swap);
}

std::string formatNmea2000Data(
    google::protobuf::uint64 value, bool swap, int w = 2*8) {
  std::stringstream ss;

  int n = w/2;
  for (int i = 0; i < n; i++) {
    int byte = (swap ? n - i - 1 : i);
    auto x = (value >> (byte * 8)) & 0xff;

    ss << std::setfill('0')
       << std::setw(2) << std::hex
       << std::uppercase << x;
  }
  auto s = ss.str();
  CHECK(s.length() == w);
  return s;
}

std::string formatNmea2000Id(
    google::protobuf::uint64 x) {
  return formatNmea2000Data(x, true, 8);
}

template <typename T>
void outputRawSentences(
    google::protobuf::int64 sentenceId,
    const std::vector<TimeStamp>& times,
    const google::protobuf::RepeatedField<T>& values,
    vector<TimedString>* entries,
    Context* summary) {
  if (times.size() != values.size()) {
    LOG(ERROR) << "The number of timestamps "
        "does not correspond to the number of values";
    return;
  }
  int n = times.size();
  auto prefix = "can0 " + formatNmea2000Id(sentenceId) + "#";
  for (int i = 0; i < n; i++) {
    entries->push_back(
        TimedString{
          times[i],
          prefix + formatNmea2000Data(values.Get(i), false)});
  }
}

void streamCat(const Nmea2000Sentences& sentences,
    vector<TimedString>* entries,
    Context* summary) {
  vector<TimeStamp> times;
  Logger::unpackTime(sentences, &times);
  if (0 < sentences.regularsizesentences_size() &&
      0 < sentences.oddsizesentences_size()) {
    LOG(ERROR) << "The Nmea2000Sentences object cannot contain"
        "both regular- and odd size sentences at the same time.";
    return;
  }
  auto id = sentences.sentence_id();
  if (0 < sentences.regularsizesentences_size()) {
    outputRawSentences(
        id, times, sentences.regularsizesentences(),
        entries, summary);
  } else if (0 < sentences.oddsizesentences_size()) {
    outputRawSentences(
        id, times, sentences.regularsizesentences(),
        entries, summary);
  }
  summary->addSummary("raw NMEA 2000", formatNmea2000Id(id), times);
}

void dispEntries(std::vector<TimedString> *entries,
    DateFormat fmt) {
  std::sort(entries->begin(), entries->end());
  for (const TimedString& entry : *entries) {
    switch (fmt) {
      case DateFormat::HumanReadable: {
        cout << entry.time.fullPrecisionString() << ": ";
        break;
      };
      case DateFormat::Nmea2000Replay: {
        cout << std::fixed << std::setprecision(3)
         <<  "(" << 0.001*entry.time.toMilliSecondsSince1970() << ") ";
        break;
      }
    };
    cout << entry.str << endl;
  }
}

void logCat(const LogFile& data, Context *summary) {
  // Always display this data, because
  // it doesn't take up much space
  if (summary->withHeader) {
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
  }

  // Visit all the data
  // of the log file and accumulate it
  vector<TimedString> entries;
  if (summary->withStreams) {
    for (int i = 0; i < data.stream_size(); ++i) {
      streamCat(data.stream(i), &entries, summary);
    }
  }

  if (summary->withText) {
    for (int i = 0; i < data.text_size(); ++i) {
      streamCat(data.text(i), &entries, summary);
    }
  }

  if (summary->withNmea2000) {
    for (int i = 0; i < data.rawnmea2000_size(); ++i) {
      streamCat(data.rawnmea2000(i), &entries, summary);
    }
  }

  if (summary->fullReport()) {
    dispEntries(&entries, summary->dateFormat);
  }
}

void catField(const LogFile& data,
    const std::string& field,
    Context *summary) {
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
  if (summary->fullReport()) {
    dispEntries(&entries, summary->dateFormat);
  }
}

void logCat(const std::string& file,
    const std::string& textField,
    Context *summary) {

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

  cmdLine.registerOption("-b", "A briefReport summary")
      .setUnique();

  cmdLine.registerOption(
      "--raw-nmea2000",
      "Only raw NMEA 2000 data, formatted to be replayed.");

  if (cmdLine.parse(argc, argv) != ArgMap::Continue) {
    return -1;
  }

  if (cmdLine.optionProvided("-b")) {
    summaryThreshold = std::numeric_limits<double>::infinity();
  }

  Array<ArgMap::Arg*> files = cmdLine.freeArgs();
  sort(files.begin(), files.end(), LexicalOrder());

  Context summary(summaryThreshold*1.0_s);
  if (cmdLine.optionProvided("--raw-nmea2000")) {
    summary.withHeader = false;
    summary.withStreams = false;
    summary.withText = false;
    summary.withNmea2000 = true;
    summary.dateFormat = DateFormat::Nmea2000Replay;
  }
  for (auto arg : cmdLine.freeArgs()) {
    logCat(arg->value(), textField, &summary);
  }
  if (summary.briefReport()) {
    std::cout << summary << std::endl;
  }
  return 0;
}

