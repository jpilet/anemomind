/*
 * AstraLoader.h
 *
 *  Created on: 9 Mar 2018
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_LOGIMPORT_ASTRALOADER_H_
#define SERVER_NAUTICAL_LOGIMPORT_ASTRALOADER_H_

#include <string>
#include <server/transducers/Transducer.h>
#include <server/common/Optional.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/TimeStamp.h>
#include <server/nautical/logimport/LogAccumulator.h>
#include <map>

namespace sail {

struct AstraHeader {
  std::string value;
};

Optional<AstraHeader> tryParseAstraHeader(const std::string& s);

enum AstraLogType {
  RawDinghy,
  ProcessedCoach,
  Unknown
};

struct AstraData {
  AstraLogType logType = AstraLogType::Unknown;
  TimeStamp partialTimestamp; // <-- Full timestamp, or only the day/date.
  Optional<Duration<double>> timeOfDay;

  TimeStamp fullTimestamp() const {
    if (!partialTimestamp.defined()) {
      return TimeStamp();
    }
    for (auto tod: timeOfDay) {
      return partialTimestamp + tod;
    }
    return partialTimestamp;
  }

  Optional<std::string> userId;
  Optional<int> dinghyId;
  Optional<Angle<double>> lat;
  Optional<Angle<double>> lon;
  Optional<Angle<double>> pitch;
  Optional<Angle<double>> roll;
  Optional<Angle<double>> COG;
  Optional<Velocity<double>> SOG;

  Optional<Angle<double>> GWD, TWD;
  Optional<Velocity<double>> GWS, TWS;

  Optional<GeographicPosition<double>> geoPos() const {
    for (auto lon0: lon) {
      for (auto lat0: lat) {
        return GeographicPosition<double>(lon0, lat0);
      }
    }
    return {};
  }
};

typedef std::function<bool(std::string, AstraData*)> AstraValueParser;

typedef Array<std::pair<
    std::string, AstraValueParser>> AstraColSpec;
typedef Array<std::string> AstraTableRow;

Optional<AstraColSpec> tryParseAstraColSpec(const Array<std::string>& s);

Array<std::string> tokenizeAstra(const std::string& s);

Optional<Duration<double>> tryParseAstraTimeOfDay(const std::string& s);

Optional<TimeStamp> tryParseAstraDate(const std::string& s);

Optional<std::map<std::string, Array<std::string>>> tryParseNamedParameters(
    const std::string& s);


struct AstraLinePreparseStep : public StatelessStepper {
  template <typename R>
  void apply(R* result, std::string& s) {
    for (const auto& h: tryParseAstraHeader(s)) {
      result->add(h);
      return;
    }
    auto tokens = tokenizeAstra(s);
    for (const auto& sp: tryParseAstraColSpec(tokens)) {
      result->add(sp);
      return;
    }
    result->add(tokens);
  }
};

inline GenericTransducer<AstraLinePreparseStep> trPreparseAstraLine() {
  return genericTransducer(AstraLinePreparseStep());
}

Optional<AstraData> tryMakeAstraData(
    const AstraColSpec& cols,
    const AstraTableRow& rawData,
    bool verbose);

void displayColHint(const AstraTableRow& rawData);

// Used to guess the type of log we are importing
bool isDinghyLogHeader(const std::string& s);
bool isProcessedCoachLogHeader(const std::string& s);

/**
 * Transforms tokens into
 */
class AstraDataStepper : public StatelessStepper {
public:

  /**
   *  Called when a new column specification is parsed,
   *  explaining what each column means.
   */
  template <typename R>
  void apply(R* result, const AstraColSpec& spec) {
    _spec = spec;
  }

  /// Called for headers starting with '-----'
  template <typename R>
  void apply(R* result, const AstraHeader& h) {
    if (isDinghyLogHeader(h.value)) {
      _type = AstraLogType::RawDinghy;
    } else if (isProcessedCoachLogHeader(h.value)) {
      _type = AstraLogType::ProcessedCoach;
    }
  }

  /// Called for tokenized table rows.
  template <typename R>
  void apply(R* result, const AstraTableRow& row) {
    auto cols = tryMakeAstraData(_spec, row, _verbose);
    if (cols.defined()) {
      auto r = cols.get();
      r.logType = _type;
      result->add(r);
    } else if (_spec.empty() && _verbose) {
      displayColHint(row);
      _verbose = false;
    }
  }
private:
  AstraLogType _type;
  bool _verbose = true;
  AstraColSpec _spec;
};

inline GenericTransducer<AstraDataStepper> trMakeAstraData() {
  return genericTransducer(AstraDataStepper());
}

Array<AstraData> loadAstraFile(const std::string& filename);

bool accumulateAstraLogs(const std::string& filename, LogAccumulator* dst);

} /* namespace sail */

#endif /* SERVER_NAUTICAL_LOGIMPORT_ASTRALOADER_H_ */
