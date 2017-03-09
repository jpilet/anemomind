/*
 * SmoothGPSFilter.cpp
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#include <server/common/ArrayBuilder.h>
#include <server/common/Functional.h>
#include <server/common/logging.h>
#include <server/math/nonlinear/CeresTrajectoryFilter.h>
#include <server/math/Resampler.h>
#include <server/nautical/filters/GpsUtils.h>
#include <server/nautical/filters/SmoothGpsFilter.h>
#include <server/common/Progress.h>
#include <server/common/TimedTypedefs.h>
#include <server/common/Span.h>
#include <server/common/DOMUtils.h>
#include <server/nautical/WGS84.h>

namespace sail {

namespace {
  template <typename Range>
  Array<TimeStamp> getTimeStamps(const Range &r) {
    ArrayBuilder<TimeStamp> dst(r.end() - r.begin());
    for (auto x: r) {
      dst.add(x.time);
    }
    return dst.get();
  }

  Array<TimeStamp> buildSampleTimes(
      const Array<TimeStamp> &positionTimes,
      const Array<TimeStamp> &motionTimes, Duration<double> period) {
    // we actually do not care about motionTimes:
    // we need to know the position. We'll compute the motion later if
    // it is missing.
    // If we have only the motion, then we cut out.
    return Resampler::resample(positionTimes, period);
  }

  Array<CeresTrajectoryFilter::Types<2>::TimedPosition> getLocalPositions(
      const GeographicReference &geoRef,
      const Array<TimedValue<GeographicPosition<double> >> &positions) {
    int n = positions.size();
    Array<CeresTrajectoryFilter::Types<2>::TimedPosition> dst(n);
    Progress prog(n);

    for (int i = 0; i < n; i++) {
      auto &y = dst[i];
      auto x = positions[i];
      y.time = x.time;
      y.value = geoRef.map(x.value);
      if (prog.endOfIteration()) {
        LOG(INFO) << prog.iterationMessage();
      }
    }
    return dst;
  }
}


TimedSampleCollection<GeographicPosition<double> >::TimedVector
  LocalGpsFilterResults::getGlobalPositions() const {
  int n = filteredLocalPositions.size();
  TimedSampleCollection<GeographicPosition<double> >::TimedVector dst;
  dst.resize(n);
  for (int i = 0; i < n; i++) {
    const auto &x = filteredLocalPositions[i];
    auto &y = dst[i];
    y.time = x.time;
    y.value = geoRef.unmap(x.value);
  }
  return dst;
}

namespace {
  HorizontalMotion<double> computeHorizontalMotion(
      const Vectorize<Length<double>, 2> &left,
      const Vectorize<Length<double>, 2> &right,
      Duration<double> dur) {
    return HorizontalMotion<double>((right[0] - left[0])/dur,
                                    (right[1] - left[1])/dur);
  }
}

TimedSampleCollection<HorizontalMotion<double> >::TimedVector
  LocalGpsFilterResults::getGpsMotions(Duration<double> maxTimeDiff) const {
  int n = filteredLocalPositions.size() - 1;
  TimedSampleCollection<HorizontalMotion<double> >::TimedVector samples;
  for (int i = 0; i < n; i++) {
    const auto &left = filteredLocalPositions[i];
    const auto &right = filteredLocalPositions[i+1];
    Duration<double> timeDiff = right.time - left.time;
    if (timeDiff < maxTimeDiff) {
      TimeStamp middleTime = left.time + 0.5*timeDiff;
      HorizontalMotion<double> motion = computeHorizontalMotion(left.value, right.value, timeDiff);
      samples.push_back(TimedValue<HorizontalMotion<double> >(middleTime, motion));
    }
  }
  return samples;
}

typedef CeresTrajectoryFilter::Types<2> FTypes;

Array<FTypes::TimedMotion> toLocalMotions(
    const Array<TimedValue<HorizontalMotion<double>>> &motions) {
  int n = motions.size();
  Array<FTypes::TimedMotion> dst(n);
  for (int i = 0; i < n; i++) {
    const auto &m = motions[i];
    dst[i] = FTypes::TimedMotion(
        m.time, FTypes::Motion{m.value[0], m.value[1]});
  }
  return dst;
}

CeresTrajectoryFilter::Settings makeDefaultOptSettings() {
  CeresTrajectoryFilter::Settings settings;
  settings.huberThreshold = Length<double>::meters(12.0); // Sort of inlier threshold on the distance in meters
  settings.regWeight = 10.0;


  // Ceres can only solve smooth unconstrained problems.
  // I tried to implement inequality constraints by
  // putting a big penalty on speeds that are too high, but it doesn't really work...


  return settings;
}

Array<CeresTrajectoryFilter::Types<2>::TimedPosition> removePositionsFarAway(
    const Array<CeresTrajectoryFilter::Types<2>::TimedPosition> &src,
    Length<double> maxLen) {
  ArrayBuilder<CeresTrajectoryFilter::Types<2>::TimedPosition> dst;
  for (auto x: src) {
    auto p = x.value;
    if (sqr(double(p[0]/maxLen)) + sqr(double(p[1]/maxLen)) < 1.0) {
      dst.add(x);
    }
  }
  return dst.get();
}

LocalGpsFilterResults solveGpsSubproblem(
    const Array<TimeStamp> &samplingTimes,
    const Array<TimedValue<GeographicPosition<double>>> rawPositions,
    const Array<TimedValue<HorizontalMotion<double>>> &motions,
    const GpsFilterSettings &settings,
    DOM::Node *dst) {

  auto referencePosition = GpsUtils::getReferencePosition(rawPositions);
  GeographicReference geoRef(referencePosition);

  auto rawLocalPositions = getLocalPositions(geoRef, rawPositions);


  IndexableWrap<Array<TimeStamp>, TypeMode::ConstRef> times =
        wrapIndexable<TypeMode::ConstRef>(samplingTimes);

    Duration<double> dur = samplingTimes.last() - samplingTimes.first();


    auto maxSpeed = Velocity<double>::knots(200.0);

    // Since the geographic reference is located at the spatial median, where most of the points
    // are, we can reject point whose local coordinates are too far away from that. This will
    // probably work in most cases.
    auto filteredRawPositions = removePositionsFarAway(rawLocalPositions, dur*maxSpeed);

    if (filteredRawPositions.empty()) {
      DOM::addSubTextNode(dst, "p",
          stringFormat("Duration: %s", dur.str().c_str()));
      DOM::addSubTextNode(dst, "p",
          "Filtered raw positions is empty!").warning();
    }

    IndexableWrap<Array<FTypes::TimedPosition>, TypeMode::Value> localPositions
      = wrapIndexable<TypeMode::Value>(filteredRawPositions);

    IndexableWrap<Array<FTypes::TimedMotion>, TypeMode::Value> localMotions
      = wrapIndexable<TypeMode::Value>(toLocalMotions(motions));

    using namespace CeresTrajectoryFilter;

    typedef FTypes::TimedPosition TimedPosition;
    typedef FTypes::TimedMotion TimedMotion;

    const AbstractArray<TimeStamp> &t = times;
    const AbstractArray<TimedPosition> &p = localPositions;
    const AbstractArray<TimedMotion> &m = localMotions;

    auto e = EmptyArray<FTypes::Position>();

    Types<2>::TimedPositionArray filtered = CeresTrajectoryFilter::filter<2>(
        t, p, m,
        settings.ceresSettings, e);

    if (filtered.empty()) {
      LOG(ERROR) << "Failed to filter GPS data";
      DOM::addSubTextNode(
          dst, "p", "Failed to filter GPS data")
        .warning();
      return LocalGpsFilterResults();
    }

    return LocalGpsFilterResults{geoRef,
      rawLocalPositions,
      filtered};
}

Array<TimeStamp> listSplittingTimeStamps(const Array<TimeStamp> &timeStamps,
    Duration<double> threshold) {
  if (threshold <= Duration<double>::seconds(0.0)) {
    return Array<TimeStamp>();
  }

  ArrayBuilder<TimeStamp> builder;
  int n = timeStamps.size() - 1;
  for (int i = 0; i < n; i++) {
    auto from = timeStamps[i];
    auto dur = timeStamps[i+1] - from;
    if (dur > threshold) {
      builder.add(from + 0.5*dur);
    }
  }
  return builder.get();
}

// This function abstract *how* element should be added.
// It *should* always be added, even if it empty.
template <typename T>
void addToArrayPolicy(ArrayBuilder<Array<T> > *dst, const Array<T> &x) {
  dst->add(x);
}

// *always* returns ```splits.size() + 1``` slices of ```src```
// Slices may be empty. That is important, because we apply this
// function to several arrays and we want to align the slices of the
// different arrays.
template <typename T>
Array<Array<T> > applySplits(const Array<T> &src,
    const Array<TimeStamp> &splits) {
  ArrayBuilder<Array<T>> dst;
  int from = 0;
  for (auto splittingTime: splits) {
    int to = from;
    while (to < src.size() && src[to] < splittingTime) {
      to++;
    }
    addToArrayPolicy<T>(&dst, src.slice(from, to));
    from = to;
  }
  addToArrayPolicy<T>(&dst, src.slice(from, src.size()));
  auto result = dst.get();
  assert(result.size() == splits.size()+1);
  return result;
}

// Force instantiation so that we can unit test this code
// without having to expose the whole template in the header.
template Array<Array<TimedValue<int> > >
  applySplits<TimedValue<int>>(const Array<TimedValue<int> > &src,
    const Array<TimeStamp> &splits);

GpsFilterResults mergeSubResults(
    const std::vector<LocalGpsFilterResults> &subResults,
    Duration<double> thresh) {
  if (subResults.empty()) {
    return GpsFilterResults();
  }

  TimedSampleCollection<GeographicPosition<double> >::TimedVector positions;
  TimedSampleCollection<HorizontalMotion<double> >::TimedVector motions;

  for (auto x: subResults) {
    auto pos = x.getGlobalPositions();
    auto mot = x.getGpsMotions(thresh);
    for (auto y: pos) {
      positions.push_back(y);
    }
    for (auto y: mot) {
      motions.push_back(y);
    }
  }

  return GpsFilterResults{
    positions, motions
  };
}

Span<TimeStamp> getTimeSpan(const Array<TimeStamp> &times) {
  return Span<TimeStamp>(times.first(), times.last());
}

void splitTimeSpan(const Span<TimeStamp> &span,
    Duration<double> idealDur,
    ArrayBuilder<TimeStamp> *dst) {
  auto dur = span.maxv() - span.minv();
  if (0 < dur.seconds()) {
    int sliceCount = std::max(1, int(ceil(dur/idealDur)));
    int splitCount = sliceCount - 1;
    if (0 < splitCount) {
      Duration<double> step = (1.0/sliceCount)*dur;
      for (int i = 1; i <= splitCount; i++) {
        dst->add(span.minv() + double(i)*step);
      }
    }
  }
}

Array<TimeStamp> listSplittingTimeStampsNotTooLong(
    const Array<TimeStamp> &times, Duration<double> threshold,
    Duration<double> problemDur) {
  auto splits0 = listSplittingTimeStamps(times, threshold);
  auto preliminaryTimeSlices = applySplits(times, splits0);

  ArrayBuilder<TimeStamp> splitsBuilder;
  for (auto x: splits0) {
    splitsBuilder.add(x);
  }
  for (auto slice: preliminaryTimeSlices) {
    if (!slice.empty()) {
      auto span = getTimeSpan(slice);
      splitTimeSpan(span, problemDur, &splitsBuilder);
    }
  }

  auto splits = splitsBuilder.get();
  std::sort(splits.begin(), splits.end());
  return splits;
}

void dispSubResults(
  const LocalGpsFilterResults &x,
  DOM::Node *dstOL) {
  std::stringstream ss;
  ss << x.filteredLocalPositions.size()
      << " position samples from " << x.filteredLocalPositions.first().time
      << " to "<< x.filteredLocalPositions.last().time;
  DOM::addSubTextNode(dstOL, "p", ss.str()).success();
}

template <typename T>
Array<TimedValue<T>> toArray(const TimedSampleRange<T> &src) {
  int n = src.size();
  Array<TimedValue<T>> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = src[i];
  }
  return dst;
}

auto distanceSplitThreshold = 100.0_m;
auto relativeMinimumSampleSizePerDistanceSplit = 0.2;
auto unreliableTimeMargin = 1.0_minutes;

bool shouldSplit(
    const TimedValue<GeographicPosition<double>> &a,
    const TimedValue<GeographicPosition<double>> &b) {
  return sail::distance(a.value, b.value) > distanceSplitThreshold;
}

Length<double> computeMaxGap(
    const Array<TimedValue<GeographicPosition<double>>> &mg) {
  auto g = 0.0_m;
  for (int i = 0; i < mg.size()-1; i++) {
    g = std::max(g, sail::distance(mg[i].value, mg[i+1].value));
  }
  return g;
}

struct PositionPrefiltering {
  Array<TimedValue<GeographicPosition<double>>> goodPositions;
  Array<Span<TimeStamp>> unreliableSpans;
};

template <typename T>
void append(ArrayBuilder<T> *dst, const Array<T> &src) {
  for (auto x: src) {
    dst->add(x);
  }
}

PositionPrefiltering prefilterPositions(
    const Array<TimedValue<GeographicPosition<double>>> &src,
    DOM::Node *log) {

  DOM::addSubTextNode(log, "h3", "Filter from "
      + src.first().time.toString()
      + " to " + src.last().time.toString());

  auto body = DOM::makeSubNode(log, "pre");

  std::vector<int> splitInds;
  splitInds.push_back(0);
  for (int i = 1; i < src.size(); i++) {
    auto a = src[i-1];
    auto b = src[i];
    if (shouldSplit(a, b)) {
      DOM::addLine(&body,
          stringFormat("Gap of %.3g meters at index %d",
              sail::distance(a.value, b.value).meters(), i));
      splitInds.push_back(i);
    }
  }
  DOM::addLine(&body,
      stringFormat("Sample count: %d", src.size()));
  splitInds.push_back(src.size());
  int segmentCount = splitInds.size()-1;
  ArrayBuilder<TimedValue<GeographicPosition<double>>> accepted;
  ArrayBuilder<TimeStamp> rejected;
  for (int i = 0; i < segmentCount; i++) {
    int from = splitInds[i];
    int to = splitInds[i+1];
    double relSize = double(to - from)/src.size();
    DOM::addLine(&body,
        stringFormat(" Segment of relative size %d", relSize));
    auto sub = src.slice(from, to);
    if (relativeMinimumSampleSizePerDistanceSplit < relSize) {
      DOM::addLine(&body, " -- accept");
      append(&accepted, sub);
    } else {
      DOM::addLine(&body, " -- reject");
      for (auto x: sub) {
        rejected.add(x.time);
      }
    }
  }
  auto result = accepted.get();
  DOM::addLine(&body,
      stringFormat("MAX GAP in filtered: %.3g",
      computeMaxGap(result).meters()));
  return {result, sail::Resampler::makeContinuousSpans(
      rejected.get(), unreliableTimeMargin)};
}

Array<TimedValue<GeographicPosition<double>>> getGoodPositions(
    const PositionPrefiltering &src) {
  return src.goodPositions;
}

Array<Span<TimeStamp>> getUnreliableSpans(
    const PositionPrefiltering &src) {
  return src.unreliableSpans;
}

bool isCovered(TimeStamp t, const Array<Span<TimeStamp>> &spans) {
  if (spans.empty()) {
    return false;
  } else if (spans.size() == 1) {
    return spans.first().contains(t);
  } else {
    auto middle = spans.size()/2;
    return isCovered(t, spans[middle-1].maxv() < t?
        spans.sliceFrom(middle) : spans.sliceTo(middle));
  }
}

Array<TimedValue<HorizontalMotion<double>>> maskUnreliable(
    const Array<Span<TimeStamp>> &unreliableSpans,
    const Array<TimedValue<HorizontalMotion<double>>> &src,
    DOM::Node *log) {

  DOM::addSubTextNode(log, "h3", "Mask unreliable");
  DOM::addSubTextNode(log, "p", "Unreliable spans");
  for (auto sp: unreliableSpans) {
    DOM::addSubTextNode(log, "p",
        "  * " + sp.minv().toString()
        + " to " + sp.maxv().toString());
  }
  DOM::addSubTextNode(log, "p",
      stringFormat(" in total %d", unreliableSpans.size()));

  ArrayBuilder<TimedValue<HorizontalMotion<double>>> dst(src.size());
  for (auto x: src) {
    if (!isCovered(x.time, unreliableSpans)) {
      dst.add(x);
    }
  }
  auto result = dst.get();
  if (result.size() < src.size()) {
    DOM::addSubTextNode(log, "p",
        stringFormat("Kept %d of %d samples", result.size(), src.size()));
  } else {
    DOM::addSubTextNode(log, "p",
        stringFormat("Kept %d all samples", result.size()));
  }
  return result;
}

GpsFilterResults filterGpsData(
    const NavDataset &ds,
    DOM::Node *log,
    const GpsFilterSettings &settings) {

  DOM::addSubTextNode(log, "h2", "Filter GPS data");

  if (ds.isDefaultConstructed()) {
    LOG(WARNING) << "Nothing to filter";
    return GpsFilterResults();
  }

  //auto motions = Array<TimedValue<HorizontalMotion<double>>>(); //
  auto motions = GpsUtils::getGpsMotions(ds);
  auto positions = ds.samples<GPS_POS>();
  if (positions.empty()) {
    LOG(ERROR) << "No GPS positions in dataset, cannot filter";
    return GpsFilterResults();
  }

  auto positionTimes = getTimeStamps(positions);
  auto motionTimes = getTimeStamps(motions);

  auto samplingTimes = buildSampleTimes(positionTimes, motionTimes,
      settings.samplingPeriod);

  auto splits = listSplittingTimeStampsNotTooLong(samplingTimes,
      settings.subProblemThreshold, settings.subProblemLength);

  CHECK(std::is_sorted(splits.begin(), splits.end()));

  int expectedSliceCount = splits.size() + 1;
  auto timeSlices = applySplits(samplingTimes, splits);
  auto prefilteredPositions = map(
      applySplits(toArray(positions), splits),
      [&](const Array<TimedValue<GeographicPosition<double>>> &positions) {
        return prefilterPositions(positions, log);
  }).toArray();
  auto positionSlices = map(prefilteredPositions, &getGoodPositions)
      .toArray();
  auto unreliableSpans = map(prefilteredPositions, &getUnreliableSpans)
      .toArray();

  auto motionPreSplits = applySplits(motions, splits);
  Array<Array<TimedValue<HorizontalMotion<double>>>>
    motionSlices(expectedSliceCount);
  CHECK(motionPreSplits.size() == expectedSliceCount);
  CHECK(unreliableSpans.size() == expectedSliceCount);
  for (int i = 0; i < expectedSliceCount; i++) {
    motionSlices[i] = maskUnreliable(
        unreliableSpans[i],
        motionPreSplits[i], log);
  }


  std::cout << "Number of slices: "<< motionSlices.size() << std::endl;

  CHECK(expectedSliceCount == timeSlices.size());
  CHECK(expectedSliceCount == positionSlices.size());
  CHECK(expectedSliceCount == motionSlices.size());
  std::vector<LocalGpsFilterResults> subResults;

  DOM::addSubTextNode(log, "h2", "Producing GPS filter sub results");
  auto ol = DOM::makeSubNode(log, "ol");

  subResults.reserve(expectedSliceCount);
  for (int i = 0; i < expectedSliceCount; i++) {
    auto timeSlice = timeSlices[i];
    auto positionSlice = positionSlices[i];
    auto motionSlice = motionSlices[i];
    auto li = DOM::makeSubNode(&ol, "li");

    DOM::addSubTextNode(&li, "p",
        stringFormat("Input: %d times, %d positions, %d motions",
        timeSlice.size(), positionSlice.size(), motionSlice.size()));

    if (2 <= timeSlice.size() && !positionSlice.empty()) {
       auto subResult = solveGpsSubproblem(
           timeSlice, positionSlice,
           motionSlice, settings, &li);
       if (!subResult.empty()) {
         subResults.push_back(subResult);
         dispSubResults(subResult, &ol);
       } else {
         DOM::addSubTextNode(&li, "p", "Empty results")
           .warning();
       }
    } else {
      DOM::addSubTextNode(&li, "p", "Too few positions or times")
        .warning();
    }
  }
  return mergeSubResults(subResults, settings.subProblemThreshold);
}

}
