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
#include <server/nautical/filters/SmoothGpsFilter.h>
#include <server/nautical/BoatState.h>
#include <server/nautical/calib/BoatStateReconstructor.h>
#include <server/common/TimedValueCutter.h>
#include <server/common/logging.h>

namespace sail {
namespace Processor2 {

struct CutVisitor {
  Array<CalibDataChunk> &_chunks;
  Array<Span<TimeStamp>> _timeSpans;

  CutVisitor(Array<CalibDataChunk> *chunks,
      const Array<Span<TimeStamp>> &timeSpans) :
    _chunks(*chunks),
    _timeSpans(timeSpans) {}

  template <DataCode Code, typename T>
  void visit(
      const char *shortName,
      const std::string &sourceName,
    const std::shared_ptr<DispatchData> &raw,
    const TimedSampleCollection<T> &coll) {
    auto cut = cutTimedValues(
        coll.samples().begin(), coll.samples().end(),
        _timeSpans);
    CHECK(cut.size() == _chunks.size());
    CHECK(cut.size() == _timeSpans.size());
    for (int i = 0; i < _chunks.size(); i++) {
      (*ChannelFieldAccess<Code>::get(_chunks[i]))[sourceName] = cut[i];
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
  stream << "Filtered GPS positions: "
      << getSpanString(chunk.filteredPositions) << std::endl;
#define DISP_CHANNELS(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  stream << SHORTNAME << ":\n"; for (auto kv: chunk.HANDLE) { \
    stream << "  " << kv.first << ": " << getSpanString(kv.second);\
  }
FOREACH_CHANNEL(DISP_CHANNELS)
#undef DISP_CHANNELS
  return stream;
}

void outputChunkInformation(
    std::string filename,
    const Array<Span<TimeStamp>> &timeSpans,
    const Array<CalibDataChunk> &chunks) {
  std::ofstream file(filename);
  std::cout << "Number of time spans: " << timeSpans.size() << std::endl;
  CHECK(timeSpans.size() == chunks.size());
  int n = timeSpans.size();
  for (int i = 0; i < n; i++) {
    auto span = timeSpans[i];
    auto chunk = chunks[i];
    file << "==== Calib chunk " << i+1 << " of " << n << " from "
        << span.minv() << " to "
        << span.maxv() << std::endl;
    file << chunk;
    file << "\n\n\n\n";
  }
}

Array<CalibDataChunk> makeCalibChunks(
    const Array<Span<TimeStamp>> &timeSpans,
    const Array<TimedValue<GeographicPosition<double>>> &filteredPositions,
    const Dispatcher *d) {

  auto cutGpsPositions = cutTimedValues(
      filteredPositions.begin(),
      filteredPositions.end(),
      timeSpans);

  int n = timeSpans.size();
  Array<CalibDataChunk> chunks(n);
  for (int i = 0; i < n; i++) {
    chunks[i].filteredPositions = cutGpsPositions[i];
  }

  CutVisitor v(&chunks, timeSpans);
  visitDispatcherChannelsConst(d, &v);
  return chunks;
}

Array<ReconstructionResults> reconstructAllGroups(
    const Array<Spani> &calibGroups,
    const Array<Span<TimeStamp>> &smallSessions,
    const Array<TimedValue<GeographicPosition<double>>> &positions,
    const Dispatcher *d,
    const Settings &settings) {

  Array<CalibDataChunk> chunks
    = makeCalibChunks(smallSessions, positions, d);

  if (settings.debug) {
    outputChunkInformation(settings.makeLogFilename("calibchunks.txt"),
        smallSessions, chunks);
  }


  /*Reconstructor::Settings recSettings;
  recSettings.windowSize = settings.calibWindowSize;
  int n = calibGroups.size();
  Array<Reconstructor::Results> results(n);
  for (int i = 0; i < n; i++) {
    auto group = calibGroups[i];
    results[i] = Reconstructor::reconstruct(
        chunks.slice(group.minv(), group.maxv()),
        recSettings);
  }

  return results;*/
  return Array<ReconstructionResults>();
}

void runDemoOnDataset(NavDataset &dataset) {
  dataset.mergeAll();

  auto d = dataset.dispatcher().get();
  Processor2::Settings settings;

  // First of all, we are going to filter all the GPS data
  Array<TimedValue<GeographicPosition<double> > > allFilteredPositions
    = Processor2::filterAllGpsData(dataset, settings);

  // Based on the timestamps of the filtered GPS data, we will build short
  // sessions of data.

  auto filteredTimeStamps = getTimeStamps(
      allFilteredPositions.begin(),
      allFilteredPositions.end());

  auto smallSessions = Processor2::segmentSubSessions(
      filteredTimeStamps, settings.subSessionCut);

  if (settings.debug) {
    Processor2::outputTimeSpansToFile(
        settings.makeLogFilename("small_sessions.txt"),
        smallSessions);
  }

  // In order to perform calibration, we need to make sure that there is
  // enough data for every optimization problem.
  auto calibGroups = computeCalibrationGroups(
      smallSessions, settings.minCalibDur);

  if (settings.debug) {
    outputGroupsToFile(
        settings.makeLogFilename("calibration_groups.txt"),
        calibGroups,
        smallSessions);
  }

  auto reconstructions
    = reconstructAllGroups(calibGroups, smallSessions,
        allFilteredPositions, d, settings);

  if (settings.debug) {
    // TODO
  }
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


Settings::Settings() : debug(true) {
  logRoot = "/tmp/";

  subSessionCut = Duration<double>::minutes(3.0);
  mainSessionCut = Duration<double>::hours(1.0);
  minCalibDur = Duration<double>::hours(3.0);
  calibWindowSize = Duration<double>::minutes(2.0);

  sessionCutSettings.cuttingThreshold = Duration<double>::hours(1.0);
  sessionCutSettings.regularization = 1.0;
}

std::string Settings::makeLogFilename(const std::string &s) const {
  return PathBuilder::makeDirectory(logRoot)
    .makeFile(s).get().toString();
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


Array<TimedValue<GeographicPosition<double> > >
  filterAllGpsData(const NavDataset &ds, const Settings &settings) {
  auto timeStamps = getAllGpsTimeStamps(ds.dispatcher().get());
  auto timeSpans = Processor2::segmentSubSessions(
      timeStamps, settings.mainSessionCut);

  if (settings.debug) {
    outputTimeSpansToFile(
        settings.makeLogFilename("presegmented_for_gps_filter.txt"),
        timeSpans);
  }

  GpsFilterSettings gpsSettings;
  gpsSettings.subProblemThreshold = settings.subSessionCut;

  ArrayBuilder<TimedValue<GeographicPosition<double> > > dst;
  for (auto span: timeSpans) {
    auto sub = ds.slice(span.minv(), span.maxv());
    auto results = filterGpsData(sub, gpsSettings);
    auto positions = results.getGlobalPositions();
    for (auto x: positions) {
      dst.add(x);
    }
  }
  return dst.get();
}


void outputTimeSpansToFile(
    const std::string &filename,
    const Array<Span<TimeStamp> > &timeSpans) {
  std::ofstream file(filename);
  for (int i = 0; i < timeSpans.size(); i++) {
    auto s = timeSpans[i];
    file << "Span " << i+1 << " of "
        << timeSpans.size() << ": " << s.minv()
        << " to " << s.maxv() << std::endl;
  }
}

void outputGroupsToFile(
      const std::string &filename,
      const Array<Spani> &groups,
      const Array<Span<TimeStamp> > sessions) {
  std::ofstream file(filename);
  for (int i = 0; i < groups.size(); i++) {
    file << "Group " << i+1 << " of " << groups.size() << std::endl;
    auto g = groups[i];
    for (auto j: g) {
      auto span = sessions[j];
      file << "   Span from " << span.minv() << " to " << span.maxv()
          << std::endl;
    }
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
