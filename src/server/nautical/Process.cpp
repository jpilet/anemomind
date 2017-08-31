/*
 * Process.cpp
 *
 *  Created on: 28 Jul 2017
 *      Author: jonas
 */

#include "Process.h"
#include <server/common/Span.h>
#include <server/common/DynamicUtils.h>
#include <server/common/Transducer.h>
#include <server/nautical/DynamicChannelValue.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/Functional.h>
#include <boost/variant/get.hpp>
#include <server/common/FrequencyLimiter.h>
#include <device/anemobox/DispatcherUtils.h>
#include <unordered_map>
#include <unordered_set>
#include <server/common/HashUtils.h>
#include <Eigen/Dense>
#include <server/nautical/WGS84.h>
#include <server/math/SegmentOutlierFilter.h>

namespace sail {
  struct ChannelDataKey {
    DataCode code = AWA;
    Keyword source;

    ChannelDataKey(DataCode c, Keyword s) : code(c), source(s) {}

    bool operator==(const ChannelDataKey& other) const {
      return code == other.code && source == other.source;
    }
  };
}

namespace std {
  template <>
  struct hash<sail::ChannelDataKey> {
    size_t operator()(const sail::ChannelDataKey& k) const {
      using namespace sail;
      return combineHash(
          combineHash(0, computeHash(k.source)),
          computeHash(k.code));
    }
  };
}

namespace sail {

std::array<double, 3> toPos(const GeographicPosition<double>& p) {
  Length<double> xyz[3];
  WGS84<double>::toXYZ(p, xyz);
  return std::array<double, 3>{
    xyz[0].meters(),
    xyz[1].meters(),
    xyz[2].meters()
  };
}

SerializationInfo toDynamicObject(
    const DataCode& x, Poco::Dynamic::Var* dst) {
  return toDynamicObject(
      std::string(wordIdentifierForCode(x)), dst);
}

SerializationInfo fromDynamicObject(
    const Poco::Dynamic::Var& src, DataCode *x) {
  std::string s;
  auto r = fromDynamicObject(src, &s);
  auto c = wordIdentifierToCode(s);
  if (c.defined()) {
    *x = c.get();
    return SerializationStatus::Success;
  } else {
    return SerializationStatus::Failure ;
  }
}


std::vector<LogFileInfo> adjustBoundaries(
    const std::vector<LogFileInfo>& src) {
  int n = src.size();
  std::vector<LogFileInfo> dst(src);
  for (int i = 0; i < n-1; i++) {
    auto& a = dst[i];
    auto& b = dst[i+1];
    CHECK(a.medianTime <= b.medianTime); // Expecting sorted data, by median.
    auto middle = max(a.medianTime, min(b.medianTime,
        a.maxTime + 0.5*(b.minTime - a.maxTime)));
    a.maxTime = std::min(middle, a.maxTime);
    b.minTime = std::max(middle, b.minTime);
  }
  return dst;
}

Array<TimedValue<DynamicChannelValue>> loadCroppedLogFile(
    const LogFileInfo& info) {
  LogLoader loader;
  loader.load(info.filename);
  auto ds = loader.makeNavDataset();
  auto all = getDynamicValues(ds.dispatcher().get());
  ArrayBuilder<TimedValue<DynamicChannelValue>> dst(all.size());
  for (auto x: all) {
    if (info.minTime <= x.time && x.time <= info.maxTime) {
      dst.add(x);
    }
  }
  return dst.get();
}

std::function<bool(TimedValue<DynamicChannelValue>)>
  ensureChronological() {
  auto last = std::make_shared<TimeStamp>();
  return [last](TimedValue<DynamicChannelValue> x) {
    if (last->undefined() || *last <= x.time) {
      return true;
    } else {
      LOG(ERROR) << "Out-of-order data: "
          << last->toString() << " followed by "
          << x.time.toString();
      return false;
    }
  };
}

std::vector<Duration<double>> cumulativeTimes(
    const std::vector<LogFileInfo>& info) {
  std::vector<Duration<double>> dst;
  dst.reserve(info.size() + 1);
  dst.push_back(0.0_s);
  for (auto x: info) {
    dst.push_back(dst.back() + (x.maxTime - x.minTime));
  }
  return dst;
}

class LogLoaderProgress {
public:
  LogLoaderProgress(const std::vector<LogFileInfo>& info)
    : _info(info), _cumulative(cumulativeTimes(info)),
      _at(0) {}

  Duration<double> total() const {return _cumulative.back();}

  Duration<double> estimate(
      TimeStamp t) {
    while (_at < _info.size() && _info[_at].maxTime < t) {
      _at++;
    }
    return _at < _info.size()? _cumulative[_at]
          + (t - _info[_at].minTime) : total();
  }
private:
  std::vector<LogFileInfo> _info;
  std::vector<Duration<double>> _cumulative;
  int _at;
};

std::function<void(TimedValue<DynamicChannelValue>)>
  logLoaderProgress(const std::vector<LogFileInfo>& info,
      const std::string& context) {
  auto pg = std::make_shared<LogLoaderProgress>(info);
  return progressNotifier<TimedValue<DynamicChannelValue>>(
      [pg, context](
      int n, const TimedValue<DynamicChannelValue>& v) {
    auto completed = pg->estimate(v.time);
    LOG(INFO) << context << ":\n Loaded up to " << v.time.toString()
        << "\n which is " << completed.str() << "\n       of "
        << pg->total().str();
  });
}

std::unordered_map<Keyword, std::unordered_set<DataCode>>
   toKeywords(const std::map<std::string, std::set<DataCode>>& toRemove) {
  std::unordered_map<Keyword, std::unordered_set<DataCode>> dst;
  for (auto kv: toRemove) {
    dst[Keyword::make(kv.first)] = std::unordered_set<DataCode>(
        kv.second.begin(), kv.second.end());
  }
  return dst;
}

std::function<bool(TimedValue<DynamicChannelValue>)>
  dataSourceFilter(
      const std::map<std::string, std::set<DataCode>>& toRemove) {
  auto m = toKeywords(toRemove);
  return [m](const TimedValue<DynamicChannelValue>& x) {
    auto f = m.find(x.value.source());
    if (f == m.end()) {
      return true;
    }
    return f->second.count(x.value.code()) <= 0;
  };
}

std::function<bool(TimedValue<DynamicChannelValue>)>
  downsample(Duration<double> minPeriod) {
  auto m = std::make_shared<std::unordered_map<
      ChannelDataKey, TimeStamp>>();
  return [m, minPeriod](const TimedValue<DynamicChannelValue>& v) {
    ChannelDataKey k(v.value.code(), v.value.source());
    auto& y = (*m)[k];
    if (y.undefined() || y + minPeriod <= v.time) {
      y = v.time;
      return true;
    }
    return false;
  };
}

auto makeLogLoaderTransducer(
    const std::vector<LogFileInfo>& logFiles,
    const std::string& context,
    const ProcessSettings& settings)
  AUTO_EXPR(composeTransducers(
      map(&loadCroppedLogFile),
      Cat<Array<TimedValue<DynamicChannelValue>>>(),
      Filter<TimedValue<DynamicChannelValue>>(
          ensureChronological()),
      Filter<TimedValue<DynamicChannelValue>>(
          dataSourceFilter(settings.samplesToRemove)),
      Filter<TimedValue<DynamicChannelValue>>(
          downsample(
              settings.downsampleMinPeriodSeconds*1.0_s)),
      visit<TimedValue<DynamicChannelValue>>(
          logLoaderProgress(logFiles, context))));

bool sameClass(const TimedValue<int>& a, const TimedValue<int>& b) {
  return a.value == b.value;
}

std::function<bool(TimedValue<DynamicChannelValue>)>
  byGpsPosTimeGap(const Duration<double>& v) {
  auto last = std::make_shared<TimeStamp>();
  return [last, v](const TimedValue<DynamicChannelValue>& x) {
    if (x.value.code() == GPS_POS) {
      auto& l = *last;
      bool newSegment = l.undefined() || x.time - l > v;
      l = x.time;
      return newSegment;
    }
    return false;
  };
}

std::string summarize(
    const Array<TimedValue<DynamicChannelValue>>& x) {
  std::stringstream ss;
  ss << "  " << x.size() << " samples from " <<
      x.first().time.toString() << " to " <<
      x.last().time.toString();
  return ss.str();
}

bool isGpsPosition(const TimedValue<DynamicChannelValue>& x) {
  return x.value.code() == GPS_POS;
}

Array<TimedValue<DynamicChannelValue>>
  cropTrailingData(
      const Array<TimedValue<DynamicChannelValue>>& src) {
  for (int i = src.size()-1; i > 0; i--) {
    if (isGpsPosition(src[i])) {
      return src.sliceTo(i+1);
    }
  }
  return Array<TimedValue<DynamicChannelValue>>();
}

bool hasData(const Array<TimedValue<DynamicChannelValue>>& x) {
  return !x.empty();
}

struct PrefilteredSession {

};

TimedValue<GeographicPosition<double>> toTimedGpsPos(
    const TimedValue<DynamicChannelValue>& v) {
  return {v.time, v.value.get<GPS_POS>()};
}

std::function<sof::Pair<3>(
    TimedValue<GeographicPosition<double>>)> toTimedEcef(
      const TimeStamp& offset) {
  return [offset](
      const TimedValue<GeographicPosition<double>>& x) {

    std::array<Length<double>, 3> xyz0;
    WGS84<double>::toXYZ(x.value, xyz0.data());

    std::array<double, 3> xyz;
    for (int i = 0; i < 3; i++) {
      xyz[i] = xyz0[i].meters();
    }
    double t = (x.time - offset).seconds();
    return sof::Pair<3>{t, xyz};
  };
}

PrefilteredSession prefilterSession(
    const Array<TimedValue<DynamicChannelValue>>& v,
    const ProcessSettings& settings) {

  // Extract only the GPS positions as timed values.
  auto T0 = composeTransducers(
      filter(&isGpsPosition),
      Map<TimedValue<GeographicPosition<double>>,
        TimedValue<DynamicChannelValue>>(&toTimedGpsPos));

  std::vector<TimedValue<GeographicPosition<double>>> positions;
  reduceIntoCollection(T0, &positions, v);

  int n = positions.size();
  if (n == 0) {
    LOG(WARNING) << "Empty GPS positions";
    return PrefilteredSession();
  }
  auto offset = positions[n/2].time;

  // Convert the timed GPS positions to the format expected
  // by the SegmentOutlierFilter
  std::vector<sof::Pair<3>> normalizedEcefData;
  normalizedEcefData.reserve(n);
  auto T1 = map(toTimedEcef(offset));
  reduceIntoCollection(T1, &normalizedEcefData, positions);

  auto mask = sof::optimize<3>(
      Array<sof::Pair<3>>::referToVector(normalizedEcefData),
      settings.gpsOutlierSettings);

  std::vector<TimedValue<
    GeographicPosition<double>>> inlierPositions;
  inlierPositions.reserve(n);

  {
    int at = 0;
    reduceIntoCollection(
        Filter<TimedValue<GeographicPosition<double>>>(
            [&](const TimedValue<GeographicPosition<double>>& p) {
      return mask[at++];
    }), &inlierPositions, positions);
  }

  return PrefilteredSession();
}

std::function<PrefilteredSession(
    Array<TimedValue<DynamicChannelValue>>)> prefilterSession(
        const ProcessSettings& s) {
  return [s](const Array<TimedValue<DynamicChannelValue>>& v) {
    return prefilterSession(v, s);
  };
}

std::vector<Span<TimeStamp>> presegmentData(
    const std::vector<LogFileInfo>& logFiles,
    const ProcessSettings& settings,
    DOM::Node* output) {
  auto showProgress  = progressNotifier<TimedValue<
      DynamicChannelValue>>(
    [](int count,
      const TimedValue<DynamicChannelValue>& v) {
    LOG(INFO) << "Loaded up to " << v.time.toString();
  });

  typedef SpanWithCount<TimedValue<int>> TimeBounds;

  auto T = composeTransducers(
      makeLogLoaderTransducer(
          logFiles, "Presegment data",
          settings),
      Bundle<TimedValue<DynamicChannelValue>>(
          byGpsPosTimeGap(settings.timeGapMinutes*1.0_minutes)),
      Map<Array<TimedValue<DynamicChannelValue>>,
        Array<TimedValue<DynamicChannelValue>>>(&cropTrailingData),
      Filter<Array<TimedValue<DynamicChannelValue>>>(&hasData),
      Map<std::string, Array<TimedValue<DynamicChannelValue>>>(
          &summarize));

  std::vector<std::string> result;
  auto iter = std::inserter(result, result.end());
  reduce(T.apply(iteratorStep(iter)), iter, logFiles);
  for (auto x: result) {
    std::cout << x << std::endl;
  }

  return std::vector<Span<TimeStamp>>();
}


} /* namespace sail */
