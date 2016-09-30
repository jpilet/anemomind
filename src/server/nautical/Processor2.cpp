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
#include <server/nautical/filters/GpsUtils.h>
#include <server/nautical/BoatState.h>
#include <server/nautical/calib/BoatStateReconstructor.h>
#include <server/common/TimedValueCutter.h>
#include <server/common/logging.h>
#include <server/common/Functional.h>
#include <unordered_map>

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
#define DISP_CHANNELS(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  stream << SHORTNAME << ":\n"; for (auto kv: chunk.HANDLE) { \
    stream << "  " << kv.first << ": " << getSpanString(kv.second);\
  }
FOREACH_CHANNEL(DISP_CHANNELS)
#undef DISP_CHANNELS
  return stream;
}

//const Array<TimedValue<GeographicPosition<double>>> &filteredPositions,
/**/

bool isValidChunk(const CalibDataChunk &chunk) {
  return !chunk.timeMapper.empty() && !chunk.initialStates.empty() &&
      (chunk.initialStates.size() == chunk.timeMapper.sampleCount);
}

Array<HorizontalMotion<double>> computeMotionPerPosition(
    const Array<TimedValue<GeographicPosition<double>>> &pos) {
  int n = pos.size();
  assert(2 <= n);
  Array<HorizontalMotion<double>> dst(n);
  dst[0] = GpsUtils::computeHorizontalMotion(pos[0], pos[1]);
  for (int i = 1; i < n-1; i++) {
    dst[i] = GpsUtils::computeHorizontalMotion(pos[i-1], pos[i+1]);
  }
  dst[n-1] = GpsUtils::computeHorizontalMotion(pos[n-2], pos[n-1]);
  return dst;
}

Array<BoatState<double> > makeInitialStates(
    const TimeStampToIndexMapper &mapper,
    const Array<TimedValue<GeographicPosition<double>>> &positions) {
  int n = positions.size();
  auto motions = computeMotionPerPosition(positions);
  Array<BoatState<double>> states(n);
  for (int i = 0; i < n; i++) {
    states[i] = BoatState<double>{positions[i].value, motions[i]};
  }
  return states;
}

void outputChunkSummary(
    const Array<CalibDataChunk> &chunks,
    HtmlNode::Ptr log0) {
  auto table = HtmlTag::make(log0, "table");
  {
    auto tr = HtmlTag::make(table, "tr");
    HtmlTag::tagWithData(tr, "th", "Index");
    HtmlTag::tagWithData(tr, "th", "Sample count");
  }
  for (int i = 0; i < chunks.size(); i++) {
    auto tr = HtmlTag::make(table, "tr");
    HtmlTag::tagWithData(tr, "td", stringFormat("%d", i));
    HtmlTag::tagWithData(tr, "td",
        stringFormat("%d", chunks[i].timeMapper.sampleCount));
  }
}

Array<CalibDataChunk> makeCalibChunks(
    const Array<Span<TimeStamp>> &timeSpans,
    const Dispatcher *d,
    const Array<TimedValue<
      GeographicPosition<double>>> &filteredPositions,
      HtmlTag::Ptr log) {

  auto cutGpsPositions = cutTimedValues(
      filteredPositions.begin(),
      filteredPositions.end(),
      timeSpans);


  int n = timeSpans.size();
  HtmlTag::tagWithData(log, "p",
      stringFormat("Number of time spans: %d", n));
  Array<CalibDataChunk> chunks(n);
  {
    CutVisitor v(&chunks, timeSpans);
    visitDispatcherChannelsConst(d, &v);
  }{
    for (int i = 0; i < n; i++) {
      auto pos = cutGpsPositions[i];
      int stateCount = pos.size();
      if (2 <= stateCount) {
        TimeStampToIndexMapper mapper(
            pos.first().time,
            (1.0/stateCount)*(pos.last().time - pos.first().time),
            stateCount);
        chunks[i].initialStates = makeInitialStates(mapper, pos);
        chunks[i].timeMapper = mapper;
        CHECK(chunks[i].initialStates.size()
            == chunks[i].timeMapper.sampleCount);
      } else {
        HtmlTag::tagWithData(log, "p",
            stringFormat(
                "Potential problem: Too few states %d for chunk %d",
                stateCount, i));
      }
    }
  }
  if (log) {
    outputChunkSummary(chunks, log);
  }
  return chunks;
}

Array<ReconstructionResults> reconstructAllGroups(
    const Array<Spani> &calibGroups, // Indices to to the sessions
    const Array<Span<TimeStamp>> &smallSessions,
    const Array<TimedValue<GeographicPosition<double>>> &positions,
    const Dispatcher *d,
    const Settings &settings,
    HtmlNode::Ptr log) {
  HtmlTag::tagWithData(log, "h2", "Reconstruction of all groups");

  Array<CalibDataChunk> chunks
    = makeCalibChunks(smallSessions, d, positions, log);

  assert(chunks.size() == smallSessions.size());

  HtmlTag::tagWithData(log, "h3", "Reconstruction per group");
  ReconstructionSettings recSettings;
  int n = calibGroups.size();
  auto ol = HtmlTag::make(log, "ol");
  Array<ReconstructionResults> results(n);
  for (int i = 0; i < n; i++) {
    auto group = calibGroups[i];
    auto title = stringFormat("Reconstruction for group %d of %d",
                i+1, calibGroups.size());

    auto li = HtmlTag::make(ol, "li");
    auto subLog = HtmlTag::initializePage(
        HtmlTag::linkToSubPage(li, title),
        title);
    results[i] = reconstruct(chunks.slice(group.minv(), group.maxv()),
        settings.reconstructionSettings, subLog);
  }

  return results;
  return Array<ReconstructionResults>();
}

struct ChannelSummarizerVisitor {
  std::set<std::string> sources;
  std::map<DataCode, int> countPerChannel;
  std::map<std::pair<DataCode, std::string>, int> counts;
    template <DataCode Code, typename T>
  void visit(const char *shortName, const std::string &sourceName,
    const std::shared_ptr<DispatchData> &raw,
    const TimedSampleCollection<T> &coll) {
      sources.insert(sourceName);
      int n = coll.size();
      counts.insert({{Code, sourceName}, n});
      if (countPerChannel.count(Code) == 0) {
        countPerChannel[Code] = n;
      } else {
        countPerChannel[Code] += n;
      }
  }
};
std::set<DataCode> usefulChannels{
  GPS_SPEED, GPS_BEARING,
  AWA, AWS,
  MAG_HEADING, WAT_SPEED,
  ORIENT
};
void outputChannelSummary(
    const Dispatcher *src,
    HtmlNode::Ptr dst) {
    ChannelSummarizerVisitor visitor;
    visitDispatcherChannelsConst(src, &visitor);

    auto codeVector = getAllDataCodes();

    std::vector<std::string> sourceVector(
        visitor.sources.begin(), visitor.sources.end());
    renderTable(dst,
        1 + visitor.sources.size() + 1,
        1 + codeVector.size(),
        [](int i, int j) {return i == 0;},
        [&](HtmlNode::Ptr e, int i, int j) {
          if (j == 0) {
            if (i == 0) {
              e->stream() << "Source";
            } else if (i-1 < sourceVector.size()) {
              e->stream() << sourceVector[i-1];
            } else {
              e->stream() << "TOTAL";
            }
          } else {
            DataCode code = codeVector[j-1];
            if (i == 0) {
              e->stream() << wordIdentifierForCode(code);
            } else if (i-1 < sourceVector.size()) {
              auto source = sourceVector[i-1];
              auto f = visitor.counts.find({code, source});
              if (f != visitor.counts.end()) {
                e->stream() << f->second;
              }
            } else {
              auto f = visitor.countPerChannel.find(code);
              if (f != visitor.countPerChannel.end()) {
                e->stream() << f->second;
              }
            }
          }
        });
    bool allGood = true;
    for (auto code: usefulChannels) {
      if (visitor.countPerChannel.count(code) == 0) {
        allGood = false;
        HtmlTag::tagWithData(dst, "p",
            {{"class", "warning"}},
            stringFormat("Channel of type %s is missing",
                wordIdentifierForCode(code)));
      }
    }
    if (allGood) {
      HtmlTag::tagWithData(dst, "p",
          {{"class", "success"}},
          "All channels of interest are present");
    }
}

void runDemoOnDataset(
    NavDataset &dataset,
    HtmlNode::Ptr dstLogNode) {
  auto logBody = HtmlTag::initializePage(
      dstLogNode, "V2 Reconstruction results");
  HtmlTag::tagWithData(logBody, "h1", "Results summary");

  if (logBody) {
    HtmlTag::tagWithData(logBody, "h2", "Channel summary");
    outputChannelSummary(
        dataset.dispatcher().get(),
        logBody);
  }
  dataset.mergeAll();

  auto d = dataset.dispatcher().get();
  Processor2::Settings settings;

  HtmlTag::tagWithData(logBody, "p",
      "First of all, we are going to filter all the GPS data");
  Array<TimedValue<GeographicPosition<double> > > allFilteredPositions
    = Processor2::filterAllGpsData(dataset, settings, logBody);

  HtmlTag::tagWithData(logBody, "p",
      "Based on the timestamps of the filtered GPS data, "
      "we will build short sessions of data.");
  auto filteredTimeStamps = getTimeStamps(
      allFilteredPositions.begin(),
      allFilteredPositions.end());

  auto smallSessions = Processor2::segmentSubSessions(
      filteredTimeStamps, settings.subSessionCut);

  if (logBody) {
    HtmlTag::tagWithData(logBody, "h2", "Small sessions");
    outputTimeSpans(smallSessions, logBody);
  }

  HtmlTag::tagWithData(logBody, "p",
    "In order to perform calibration, we need to make sure that there is"
    "enough data for every optimization problem, so we will form groups");
  auto calibGroups = computeCalibrationGroups(
      smallSessions, settings.minCalibDur);

  if (logBody) {
    HtmlTag::tagWithData(logBody, "h2", "Calibration groups");
    outputGroups(
        calibGroups,
        smallSessions, logBody);
  }

  auto reconstructions
    = reconstructAllGroups(
        calibGroups, smallSessions,
        allFilteredPositions, d, settings,
        logBody);

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
  filterAllGpsData(const NavDataset &ds, const Settings &settings,
      HtmlNode::Ptr log) {
  HtmlTag::tagWithData(log, "h2", "GPS filtering");
  HtmlTag::tagWithData(log, "h2",
      "Filtering all the GPS data with some chopping");
  auto timeStamps = getAllGpsTimeStamps(ds.dispatcher().get());
  auto timeSpans = Processor2::segmentSubSessions(
      timeStamps, settings.mainSessionCut);
  HtmlTag::tagWithData(log, "p",
      "The data is filtered with these time spans");
  outputTimeSpans(timeSpans, log);

  GpsFilterSettings gpsSettings;
  gpsSettings.subProblemThreshold = settings.subSessionCut;

  HtmlTag::tagWithData(log, "p",
      "Here is a brief summary");
  auto table = HtmlTag::make(log, "table");
  if (log) {
    auto h = HtmlTag::make(table, "tr");
    HtmlTag::tagWithData(h, "th", "Index");
    HtmlTag::tagWithData(h, "th", "Position count");
    HtmlTag::tagWithData(h, "th", "Max speed");
    HtmlTag::tagWithData(h, "th", "Duration");
    HtmlTag::tagWithData(h, "th", "Max distance to median");
  }
  ArrayBuilder<TimedValue<GeographicPosition<double> > > dst;
  int counter = 0;
  for (auto span: timeSpans) {
    auto sub = ds.slice(span.minv(), span.maxv());
    auto results = filterGpsData(sub, gpsSettings);
    auto positions = results.getGlobalPositions();
    for (auto x: positions) {
      dst.add(x);
    }
    auto row = HtmlTag::make(table, "tr");
    if (row) {
      HtmlTag::tagWithData(row, "td", stringFormat("%d", counter));
      HtmlTag::tagWithData(row, "td", stringFormat(
          "%d", positions.size()));
      auto motions = results.getGpsMotions(60.0_s);
      auto maxSpeed = 0.0_kn;
      for (auto m: motions) {
        maxSpeed = std::max(maxSpeed, m.value.norm());
      }
      HtmlTag::tagWithData(row, "td", stringFormat(
          "%.3g knots", maxSpeed.knots()));
      auto dur = span.maxv() - span.minv();
      HtmlTag::tagWithData(row, "td", dur.str().c_str());
      auto maxDistanceToMedian = 0.0_m;
      for (auto pos: results.filteredLocalPositions) {
        auto p = pos.value;
        maxDistanceToMedian = std::max(maxDistanceToMedian,
            (p[0].meters(), p[1].meters())*1.0_m);
      }
      HtmlTag::tagWithData(row, "td", stringFormat("%.3g M",
          maxDistanceToMedian.nauticalMiles()));

      counter++;
    }
  }
  return dst.get();
}


void outputTimeSpans(
    const Array<Span<TimeStamp> > &timeSpans,
    HtmlNode::Ptr dst) {
  auto table = HtmlTag::make(dst, "table");
  {
    auto tr = HtmlTag::make(table, "tr");
    {
      auto from = HtmlTag::make(tr, "th");
      from->stream() << "From time";
    }{
      auto to = HtmlTag::make(tr, "th");
      to->stream() << "To time";
    }{
      auto dur = HtmlTag::make(tr, "th");
      dur->stream() << "Duration";
    }
  }{
    for (auto span: timeSpans) {
      auto tr = HtmlTag::make(table, "tr");
      {
        auto td = HtmlTag::make(tr, "td");
        td->stream() << span.minv().toString();
      }{
        auto td = HtmlTag::make(tr, "td");
        td->stream() << span.maxv().toString();
      }{
        auto td = HtmlTag::make(tr, "td");
        auto dur = span.maxv() - span.minv();
        td->stream() << dur.str();
      }
    }
  }
}

void outputGroups(
      const Array<Spani> &groups,
      const Array<Span<TimeStamp> > sessions,
      HtmlNode::Ptr dst) {
  for (int i = 0; i < groups.size(); i++) {
    {
      auto h3 = HtmlTag::make(dst, "h3");
      h3->stream() << "Group " << i+1 <<
          " of " << groups.size() << std::endl;
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
