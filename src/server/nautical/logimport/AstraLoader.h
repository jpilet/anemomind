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

namespace sail {

struct AstraHeader {
  std::string value;
};

Optional<AstraHeader> tryParseAstraHeader(const std::string& s);

struct AstraData {
  //TimeStamp date;
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
};

typedef std::function<bool(std::string, AstraData*)> AstraValueParser;

typedef Array<std::pair<
    std::string, AstraValueParser>> AstraColSpec;
typedef Array<std::string> AstraTableRow;

Optional<AstraColSpec> tryParseAstraColSpec(const Array<std::string>& s);

Array<std::string> tokenizeAstra(const std::string& s);

Optional<Duration<double>> tryParseAstraTimeOfDay(const std::string& s);

Optional<TimeStamp> tryParseAstraDate(const std::string& s);

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
    const AstraTableRow& rawData);

class AstraDataStepper : public StatelessStepper {
public:
  template <typename R>
  void apply(R* result, const AstraColSpec& spec) {
    _spec = spec;
  }

  template <typename R>
  void apply(R* result, const AstraHeader& ) {}

  template <typename R>
  void apply(R* result, const AstraTableRow& row) {
    for (auto s: tryMakeAstraData(_spec, row)) {
      result->add(s);
    }
  }
private:
  AstraColSpec _spec;
};

inline GenericTransducer<AstraDataStepper> trMakeAstraData() {
  return genericTransducer(AstraDataStepper());
}

} /* namespace sail */

#endif /* SERVER_NAUTICAL_LOGIMPORT_ASTRALOADER_H_ */
