/*
 * Processor2.cpp
 *
 *  Created on: 18 Aug 2016
 *      Author: jonas
 */

#include "Processor2.h"
#include <server/nautical/NavDataset.h>
#include <server/common/ArrayBuilder.h>
#include <device/anemobox/DispatcherUtils.h>
#include <server/common/PathBuilder.h>
#include <server/common/PhysicalQuantityIO.h>
#include <server/common/ArrayIO.h>
#include <fstream>
#include <server/common/ReduceTree.h>
#include <server/common/logging.h>
#include <server/nautical/filters/GpsUtils.h>
#include <server/nautical/BoatState.h>
#include <server/common/TimedValueCutter.h>
#include <server/common/logging.h>
#include <server/common/Functional.h>
#include <unordered_map>
#include <server/nautical/calib/Reconstructor.h>
#include <server/html/HtmlDispatcher.h>
#include <server/nautical/filters/SplineGpsFilter.h>

namespace sail {
namespace Processor2 {

struct CutVisitor {
  Array<CalibDataChunk> &_chunks;
  Array<Span<TimeStamp>> _timeSpans;
  std::function<bool(DataCode, std::string)> _sensorFilter;
  //HtmlNode::Ptr _log;

  CutVisitor(Array<CalibDataChunk> *chunks,
      const Array<Span<TimeStamp>> &timeSpans,
      std::function<bool(DataCode, std::string)> sf) :
    _chunks(*chunks),
    _timeSpans(timeSpans),
    _sensorFilter(sf) {}

  template <DataCode Code, typename T>
  void visit(
      const char *shortName,
      const std::string &sourceName,
      const std::shared_ptr<DispatchData> &raw,
      const TimedSampleCollection<T> &coll) {
    if (_sensorFilter(Code, sourceName)) {
      auto cut = cutTimedValues(
          coll.samples().begin(), coll.samples().end(),
          _timeSpans);
      CHECK(cut.size() == _chunks.size());
      CHECK(cut.size() == _timeSpans.size());
      for (int i = 0; i < _chunks.size(); i++) {
        (*ChannelFieldAccess<Code>::get(_chunks[i]))[sourceName] = cut[i];
      }
    } else {
      /*DOM::addSubTextNode(&_log, "p", {{"class", "warning"}},
          std::string("Measured values from ") + shortName + ", "
          + sourceName + " will not be used.");*/
    }
  }
};

template <typename T>
std::string getSpanString(const Array<TimedValue<T>> &data) {
  if (data.empty()) {
    return "Empty";
  } else {
    std::stringstream ss;
    ss << data.size() << " measures from " << data.first().time << " to "
        << data.last().time << std::endl;
    return ss.str();
  }
}

std::ostream &operator<<(std::ostream &stream,
    const CalibDataChunk &chunk) {
#define DISP_CHANNELS(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  stream << SHORTNAME << ":\n"; for (auto kv: chunk.HANDLE) { \
    stream << "  " << kv.first << ": " << getSpanString(kv.second);\
  }
FOREACH_CHANNEL(DISP_CHANNELS)
#undef DISP_CHANNELS
  return stream;
}

Array<CalibDataChunk> makeCalibChunks(
    const Dispatcher *d,
    const Array<SplineGpsFilter::EcefCurve> &curves,
    std::function<bool(DataCode, std::string)> sensorFilter) {
  int n = curves.size();
  auto timeSpans = sail::map(curves,
      [](const SplineGpsFilter::EcefCurve &data) {
    return data.span();
  });
  Array<CalibDataChunk> chunks(n);
  {
    CutVisitor v(&chunks, timeSpans, sensorFilter);
    visitDispatcherChannelsConst(d, &v);
  }
  for (int i = 0; i < n; i++) {
    chunks[i].trajectory = curves[i];
  }
  return chunks;
}

Array<ReconstructionResults> reconstructAllGroups(
    const Array<Spani> &calibGroups, // Indices to to the sessions
    const Array<SplineGpsFilter::EcefCurve> &curves,
    const Dispatcher *d,
    const Settings &settings,
    DOM::Node *log,
    RNG *rng) {
  DOM::addSubTextNode(log, "h2", "Reconstruction of all groups");

  Array<CalibDataChunk> chunks
    = makeCalibChunks(
        d, curves, settings.sensorFilter);

  assert(chunks.size() == curves.size());

  DOM::addSubTextNode(log, "h3", "Reconstruction per group");
  ReconstructionSettings recSettings;
  int n = calibGroups.size();
  auto ol = DOM::makeSubNode(log, "ol");
  Array<ReconstructionResults> results(n);
  for (int i = 0; i < n; i++) {
    auto group = calibGroups[i];
    auto title = stringFormat("Reconstruction for group %d of %d",
                i+1, calibGroups.size());

    auto li = DOM::makeSubNode(&ol, "li");
    auto subLog = DOM::linkToSubPage(&li, title);
    results[i] = reconstruct(
        chunks.slice(group.minv(), group.maxv()),
        settings.reconstructionSettings, &subLog,
        rng);
  }

  return results;
}

void outputTimeSpans(
    const Array<Span<TimeStamp> > &timeSpans,
    DOM::Node *dst) {
  auto table = DOM::makeSubNode(dst, "table");
  {
    auto tr = DOM::makeSubNode(&table, "tr");
    {
      auto from = DOM::makeSubNode(&tr, "th");
      DOM::addTextNode(&from, "From time");
    }{
      auto to = DOM::makeSubNode(&tr, "th");
      DOM::addTextNode(&to, "To time");
    }{
      auto dur = DOM::makeSubNode(&tr, "th");
      DOM::addTextNode(&dur, "Duration");
    }
  }{
    for (auto span: timeSpans) {
      auto tr = DOM::makeSubNode(&table, "tr");
      {
        auto td = DOM::makeSubNode(&tr, "td");
        DOM::addTextNode(&td, span.minv().toString());
      }{
        auto td = DOM::makeSubNode(&tr, "td");
        DOM::addTextNode(&td, span.maxv().toString());
      }{
        auto td = DOM::makeSubNode(&tr, "td");
        auto dur = span.maxv() - span.minv();
        DOM::addTextNode(&td, dur.str());
      }
    }
  }
}


std::set<DataCode> usefulChannels{
  GPS_SPEED, GPS_BEARING,
  AWA, AWS,
  MAG_HEADING, WAT_SPEED,
  ORIENT
};


Array<SplineGpsFilter::EcefCurve> getFilteredGpsCurves(
    const NavDataset &ds,
    const SplineGpsFilter::Settings &settings) {
  auto posSamples = ds.samples<GPS_POS>();
  auto positions = Array<TimedValue<GeographicPosition<double>>>
      ::fromRange(posSamples.begin(), posSamples.end());
  auto motions = GpsUtils::getGpsMotions(ds);
  return SplineGpsFilter::segmentAndFilter(
      positions, motions, settings);
}

bool process(
    const Settings &settings,
    NavDataset dataset) {
  RNG rng;
  auto startTime = TimeStamp::now();

  auto d = dataset.dispatcher().get();

  auto dbOutput = settings.debugOutput;
  renderDispatcherTableOverview(d, &dbOutput);

  DOM::addSubTextNode(&dbOutput, "p",
      "First of all, we are going to filter all the GPS data");

  auto curves = getFilteredGpsCurves(dataset, settings.gpsSettings);

  auto smallSessions = sail::map(curves,
      [](const SplineGpsFilter::EcefCurve &data) {
    return data.span();
  });

    if (dbOutput) {
      DOM::addSubTextNode(&dbOutput, "h2", "Small sessions");
      outputTimeSpans(smallSessions, &dbOutput);
    }

  DOM::addSubTextNode(&dbOutput, "p",
    "In order to perform calibration, we need to make sure that there is"
    " enough data for every optimization problem, so we will form groups");
  auto calibGroups = computeCalibrationGroups(
      smallSessions, settings.minCalibDur);

  if (dbOutput) {
    DOM::addSubTextNode(&dbOutput, "h2", "Calibration groups");
    outputGroups(
        calibGroups,
        smallSessions,
        &dbOutput);
  }

  auto reconstructions
    = reconstructAllGroups(
        calibGroups,
        curves, d, settings,
        &dbOutput, &rng);

  //if (logBody) {
    //DOM::addSubTextNode(&logBody, "h2", "Summary");
    auto totalDuration = TimeStamp::now() - startTime;
    //DOM::addSubTextNode(&logBody, "p",
//        stringFormat("Processing time: %s",
//            totalDuration.str().c_str()));
  //}
  return true;
}

class TimesVisitor {
 public:

  TimesVisitor(std::function<bool(DataCode)> p) : _pred(p) {}

  ArrayBuilder<TimeStamp> times;

  template <DataCode Code, typename T>
  void visit(const char *shortName, const std::string &sourceName,
    const std::shared_ptr<DispatchData> &raw,
    const TimedSampleCollection<T> &coll) {
      if (_pred(Code)) {
        for (auto x: coll) {
          times.add(x.time);
        }
      }
  }
 private:
  std::function<bool(DataCode)> _pred;
};


Settings::Settings() {
  subSessionCut = Duration<double>::minutes(3.0);
  mainSessionCut = Duration<double>::hours(1.0);
  minCalibDur = Duration<double>::hours(3.0);

  sessionCutSettings.cuttingThreshold = Duration<double>::hours(1.0);
  sessionCutSettings.regularization = 1.0;
}

Array<TimeStamp> getAllTimeStampsFiltered(
    std::function<bool(DataCode)> pred,
    const Dispatcher *d) {
  TimesVisitor v(pred);
  visitDispatcherChannelsConst(d, &v);
  auto times =  v.times.get();
  std::sort(times.begin(), times.end());
  return times;
}

Array<TimeStamp> getAllGpsTimeStamps(const Dispatcher *d) {
  return getAllTimeStampsFiltered(
      [](DataCode c) {return c == GPS_POS;}, d);
}

void addUnique(std::vector<int> *dst, int i) {
  if (dst->empty() or dst->back() != i) {
    dst->push_back(i);
  }
}

Array<Span<TimeStamp> > segmentSubSessions(
    const Array<TimeStamp> &times,
    Duration<double> threshold) {
  if (times.empty()) {
    return Array<Span<TimeStamp> >();
  }

  std::vector<int> cuts{0};
  int n = times.size() - 1;
  for (int i = 0; i < n; i++) {
    auto dur = times[i+1] - times[i];
    if (threshold < dur) {
      cuts.push_back(i+1);
    }
  }
  addUnique(&cuts, times.size());
  ArrayBuilder<Span<TimeStamp> > spans;
  int m = cuts.size() - 1;
  for (int i = 0; i < m; i++) {
    int from = cuts[i];
    int to = cuts[i+1]-1;
    spans.add(Span<TimeStamp>(times[from], times[to]));
  }
  return spans.get();
}


void outputGroups(
      const Array<Spani> &groups,
      const Array<Span<TimeStamp> > sessions,
      DOM::Node *dst) {
  for (int i = 0; i < groups.size(); i++) {
    {
      std::stringstream ss;
      ss << "Group " << i+1 <<
          " of " << groups.size() << std::endl;
      DOM::addSubTextNode(dst, "h3", ss.str());
    }
    auto g = groups[i];
    outputTimeSpans(sessions.slice(g.minv(), g.maxv()), dst);
  }
}

namespace {
  Duration<double> dur(const Span<TimeStamp> &span) {
    return span.maxv() - span.minv();
  }

  struct SpanCand {
    int index;
    Duration<double> d;

    bool operator<(const SpanCand &other) const {
      return d < other.d;
    }
  };

  int findIndex(const ReduceTree<Spani> &tree,
      int indexToSearchFor, int nodeIndex) {
    if (tree.isLeaf(nodeIndex)) {
      return nodeIndex;
    } else {
      int leftIndex = tree.left(nodeIndex);
      Spani leftSpan = tree.getNodeValue(leftIndex);
      if (indexToSearchFor < leftSpan.minv()) {
        return -1;
      } else if (leftSpan.contains(indexToSearchFor)) {
        return findIndex(tree, indexToSearchFor, leftIndex);
      } else {
        int rightIndex = tree.right(nodeIndex);
        if (!tree.contains(rightIndex)) {
          return -1;
        } else {
          Spani rightSpan = tree.getNodeValue(rightIndex);
          if (rightSpan.contains(indexToSearchFor)) {
            return findIndex(tree, indexToSearchFor, rightIndex);
          } else {
            return -1;
          }
        }
      }
    }
  }


  Spani merge(Spani a, Spani b) {
    Spani c = a;
    c.extend(b);
    return c;
  }

  void mergeSpans(ReduceTree<Spani> *tree, int l, int r) {
    auto a = tree->getNodeValue(l);
    auto b = tree->getNodeValue(r);
    tree->setNodeValue(l, merge(a, b));
    tree->setNodeValue(r, Spani(b.maxv(), b.maxv()));
  }

  bool empty(Spani x) {
    return x.width() == 0;
  }
}

Array<Spani> groupSessionsByThreshold(
    const Array<Span<TimeStamp> > &timeSpans,
    const Duration<double> &threshold) {
  auto n = timeSpans.size();
  ArrayBuilder<Spani> dst(n);
  int from = 0;
  for (int i = 0; i < n-1; i++) {
    auto a = timeSpans[i];
    auto b = timeSpans[i+1];
    if (b.minv() - a.maxv() > threshold) {
      int to = i+1;
      dst.add(Spani(from, to));
      from = to;
    }
  }
  dst.add(Spani(from, n));
  return dst.get();
}

Array<Spani> computeCalibrationGroups(
  Array<Span<TimeStamp> > timeSpans,
  Duration<double> minCalibDur) {
  int n = timeSpans.size();
  std::cout << "Number of time spans: " << n << std::endl;
  Array<Duration<double> > cumulative(n+1);
  cumulative[0] = Duration<double>::seconds(0.0);
  Array<Spani> spans(n);
  for (int i = 0; i < n; i++) {
    cumulative[i+1] = cumulative[i] + dur(timeSpans[i]);
    spans[i] = Spani(i, i+1);
  }

  std::cout << "Built cumulative" << std::endl;

  auto dur = [&](Spani x) {
    return cumulative[x.maxv()] - cumulative[x.minv()];
  };

  ReduceTree<Spani> indexTree(&merge, spans);

  ReduceTree<Spani> durationTree(
      [&](Spani a, Spani b) {
    if (empty(a)) {
      return b;
    } else if (empty(b)) {
      return a;
    } else {
      return dur(a) < dur(b)? a : b;
    }
  }, spans);
  std::cout << "Built reduce tree" << std::endl;

  for (int i = 0; i < n-1; i++) {
    std::cout << "Iteration " << i << std::endl;
    std::cout << "Leaves: " << indexTree.leaves() << std::endl;
    auto shortest = durationTree.top();
    std::cout << "  Shortest: " << shortest << std::endl;
    auto shortestDur = dur(shortest);
    std::cout << "  Duration: " << shortestDur << std::endl;
    int shortestIndex = findIndex(indexTree, shortest.minv(), 0);
    if (minCalibDur <= shortestDur) {
      std::cout << "  long enought, we are done." << std::endl;
      break;
    }
    auto considerPair = [&](int current, int cand) {
      int candSpanIndex = findIndex(indexTree, cand, 0);
      if (candSpanIndex == -1) {
        return SpanCand{-1, Duration<double>::seconds(
            std::numeric_limits<double>::infinity())};
      } else {
        return SpanCand{candSpanIndex,
          timeSpans[std::max(current, cand)].minv() -
          timeSpans[std::min(current, cand)].maxv()};
      }
    };
    auto best = std::min(
        considerPair(shortest.minv(), shortest.minv()-1),
        considerPair(shortest.maxv()-1, shortest.maxv()));
    if (best.index == -1) {
      LOG(FATAL) << "This should not happen";
      break;
    }
    int l = std::min(shortestIndex, best.index);
    int r = std::max(shortestIndex, best.index);
    mergeSpans(&indexTree, l, r);
    mergeSpans(&durationTree, l, r);
  }
  auto leaves = indexTree.leaves();
  ArrayBuilder<Spani> nonEmpty;
  for (auto leaf: leaves) {
    if (!empty(leaf)) {
      nonEmpty.add(leaf);
    }
  }
  return nonEmpty.get();
}



}
} /* namespace sail */
