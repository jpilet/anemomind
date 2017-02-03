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

/*TimedSampleCollection<HorizontalMotion<double>>::TimedVector
  GpsFilterResults::getMotionPerPosition() const {
  int n = filteredLocalPositions.size();
  TimedSampleCollection<HorizontalMotion<double>>::TimedVector samples;
  if (n < 2) {
    LOG(WARNING) << "Too few position samples to compute motions";
    return samples;
  }
  samples.push_back(computeMotion(filteredLocalPositions[0], filteredLocalPositions[1]));
  for (int i = 1; i < n-1; i++) {
    auto timeDif =
  }
  return samples;
}*/

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
    if (from >= src.size()) {
      break;
    }
  }
  addToArrayPolicy<T>(&dst, src.slice(from, src.size()));
  return dst.get();
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

GpsFilterResults filterGpsData(
    const NavDataset &ds,
    DOM::Node *dst,
    const GpsFilterSettings &settings) {

  if (ds.isDefaultConstructed()) {
    LOG(WARNING) << "Nothing to filter";
    return GpsFilterResults();
  }



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
  auto positionSlices = applySplits(
      toArray(positions), splits);
  auto motionSlices = applySplits(motions, splits);
  CHECK(expectedSliceCount == timeSlices.size());
  CHECK(expectedSliceCount == positionSlices.size());
  CHECK(expectedSliceCount == motionSlices.size());
  std::vector<LocalGpsFilterResults> subResults;

  DOM::addSubTextNode(dst, "h2", "Producing GPS filter sub results");
  auto ol = DOM::makeSubNode(dst, "ol");

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
