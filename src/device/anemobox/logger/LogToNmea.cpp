/*
 * Convert .log files to NMEA data that Adrena can load.
 */

#include <device/anemobox/logger/Logger.h>

#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>
#include <device/anemobox/logger/LogToNav.h>
#include <logger.pb.h>
#include <server/common/ArgMap.h>
#include <server/common/logging.h>
#include <server/nautical/NavToNmea.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace sail;
using namespace std;

using namespace sail::NavCompat;

namespace {

struct TimedString {
  TimeStamp time;
  std::string str;

  TimedString(TimeStamp t, const string& s) : time(t), str(s) { }

  bool operator < (const TimedString& other) const {
    return time < other.time;
  }
};

void nmeaToTimedString(const string& block,
                       const TimeStamp& time,
                       NmeaParser* parser,
                       vector<TimedString> *sentences,
                       bool* hasRMC) {
  for (auto c: block) {
    switch (parser->processByte(c)) {
      case NmeaParser::NMEA_NONE:
        break;
      case NmeaParser::NMEA_TIME_POS:
        if (parser->sentenceType() == "RMC") {
          *hasRMC = true;
        }
      default:
        sentences->push_back(TimedString(time, parser->sentence()));
    }
  }
}

bool isGood(const Nav& nav) {
  return
    // Older than 2000? we probably have a bad time estimation.
    nav.time() > TimeStamp::date(2000, 1, 1)

    // The internal GPS returns a position of -16,0 when it does
    // not know where we are.
    && nav.geographicPosition().lat().degrees() != -16
    && nav.geographicPosition().lon().degrees() != 0;
}

void textFieldToNmeaSentences(const LogFile& data, const string& field,
                              vector<TimedString> *sentences, bool *hasRMC) {
  *hasRMC = false;
  for (int i = 0; i < data.text_size(); ++i) {
    auto valueSet = data.text(i);

    if (field == "" || valueSet.shortname() == field) {
      vector<TimeStamp> times;
      Logger::unpackTime(valueSet, &times);
      NmeaParser parser;

      for (int i = 0; i < valueSet.text_size(); ++i) {
        nmeaToTimedString(valueSet.text(i), times[i], &parser, sentences, hasRMC);
      }
    }
  }
}


void exportRMC(const LogFile& data, vector<TimedString> *sentences) {
  NavDataset navs = logFileToNavArray(data);
  if (getNavSize(navs) == 0) {
    LOG(WARNING) << "No GPS position found.";
    return;
  }

  for (const Nav& nav : Range(navs)) {
    if (!isGood(nav)) {
      continue;
    }

    sentences->push_back(TimedString(nav.time(), nmeaRmc(nav)));
    sentences->push_back(
        TimedString(nav.time() - Duration<>::milliseconds(2),
                    "$PDKI,MINILOGGER,ADRENA,MOT,0,EVT,0,E153*53"));
    sentences->push_back(
        TimedString(nav.time() - Duration<>::milliseconds(1),
                    "$PDKI,MINILOGGER,ADRENA,MOT,0,EVT,0,0*11"));
  }
}

void nmeaExport(const std::string& file, const std::string& textField) {
  LogFile data;
  if (!Logger::read(file, &data)) {
    LOG(ERROR) << file << ": can't read log file.";
    return;
  }

  vector<TimedString> sentences;
  bool hasRMC = false;
  textFieldToNmeaSentences(data, textField, &sentences, &hasRMC);
  if (!hasRMC) {
    exportRMC(data, &sentences);
  }

  sort(sentences.begin(), sentences.end());

  // Skip invalid values before 2000.
  sentences.erase(sentences.begin(),
                lower_bound(sentences.begin(), sentences.end(),
                            TimedString(TimeStamp::date(2000, 1, 1), "")));

  for (auto entry : sentences) {
    cout << entry.str << "\r\n";
  }
  cout << endl;
}

struct LexicalOrder {
  bool operator() (const ArgMap::Arg* a, const ArgMap::Arg* b) const {
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

  cmdLine.registerOption("-t", "output only the given text stream")
    .store(&textField)
    .setUnique();

  if (cmdLine.parse(argc, argv) != ArgMap::Continue) {
    return -1;
  }

  Array<ArgMap::Arg*> files = cmdLine.freeArgs();
  sort(files.begin(), files.end(), LexicalOrder());

  // Those strange sentences are proprietary. We mimick Adrena's minilogger.
  // (more or less)
  cout << "$PDKI,LOGSTART\r\n"
    "$PDKI,MINILOGGER,ADRENA,MOT,0,EVT,0,0*11\r\n"
    "$PDKI,MINILOGGER,ADRENA,MOT,0,EVT,0,0*11\r\n";

  for (auto arg : cmdLine.freeArgs()) {
    nmeaExport(arg->value(), textField);
  }
  return 0;
}

