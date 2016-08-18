/*
 * Processor2.cpp
 *
 *  Created on: 18 Aug 2016
 *      Author: jonas
 */

#include "Processor2.h"
#include <server/common/ArrayBuilder.h>
#include <device/anemobox/DispatcherUtils.h>
#include <server/common/PathBuilder.h>
#include <server/common/PhysicalQuantityIO.h>
#include <fstream>
#include <server/common/ReduceTree.h>


namespace sail {
namespace Processor2 {

class GpsTimesVisitor {
 public:
  ArrayBuilder<TimeStamp> times;

  template <DataCode Code, typename T>
  void visit(const char *shortName, const std::string &sourceName,
    const std::shared_ptr<DispatchData> &raw,
    const TimedSampleCollection<T> &coll) {
      if (Code == GPS_POS) {
        for (auto x: coll) {
          times.add(x.time);
        }
      }
  }
};


Settings::Settings() {
  logRoot = "/tmp/";

  subSessionCut = Duration<double>::minutes(3.0);
  mainSessionCut = Duration<double>::hours(1.0);
  minCalibDur = Duration<double>::hours(3.0);

  sessionCutSettings.cuttingThreshold = Duration<double>::hours(1.0);
  sessionCutSettings.regularization = 1.0;
}

std::string Settings::makeLogFilename(const std::string &s) {
  return PathBuilder::makeDirectory(logRoot)
    .makeFile(s).get().toString();
}

Array<TimeStamp> getAllGpsTimeStamps(const Dispatcher *d) {
  GpsTimesVisitor v;
  visitDispatcherChannelsConst(d, &v);
  auto times =  v.times.get();
  std::sort(times.begin(), times.end());
  return times;
}

void addUnique(std::vector<int> *dst, int i) {
  if (dst->empty() or dst->back() != i) {
    dst->push_back(i);
  }
}

Array<Span<TimeStamp> > segmentSubSessions(
    const Array<TimeStamp> &times,
    const Settings &settings) {
  if (times.empty()) {
    return Array<Span<TimeStamp> >();
  }

  std::vector<int> cuts{0};
  int n = times.size() - 1;
  for (int i = 0; i < n; i++) {
    auto dur = times[i+1] - times[i];
    if (settings.subSessionCut < dur) {
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

namespace {
  Duration<double> dur(const Span<TimeStamp> &span) {
    return span.maxv() - span.minv();
  }
}

Array<Spani> computeCalibrationGroups(
  Array<Span<TimeStamp> > timeSpans,
  Duration<double> minCalibDur) {
  int n = timeSpans.size();
  Array<Duration<double> > cumulative(n+1);
  cumulative[0] = Duration<double>::seconds(0.0);
  Array<Spani> spans(n);
  for (int i = 0; i < n; i++) {
    cumulative[i+1] = cumulative[i] + dur(timeSpans[i]);
    spans[i] = Spani(i, i+1);
  }

  auto dur = [&](Spani x) {
    return cumulative[x.maxv()] - cumulative[x.minv()];
  };

  ReduceTree<Spani> indexTree(
      [](Spani a, Spani b) {return Spani(a.minv(), b.maxv());},
      spans);
  ReduceTree<Spani> durationTree(
      [&](Spani a, Spani b) {
    return dur(a) < dur(b)? a : b;
  }, spans);
  for (int i = 0; i < n-1; i++) {
    if (minCalibDur <= dur(durationTree.top())) {
      break;
    }

  }
}



}
} /* namespace sail */
