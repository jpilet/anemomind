/*
 * SmoothGPSFilter.cpp
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#include <server/common/ArrayBuilder.h>
#include <server/common/Functional.h>
#include <server/common/logging.h>
#include <server/math/SampleUtils.h>
#include <server/nautical/filters/GpsUtils.h>
#include <server/nautical/filters/SmoothGpsFilter.h>
#include <server/common/Progress.h>
#include <server/common/TimedTypedefs.h>
#include <server/common/Span.h>
#include <server/common/DOMUtils.h>
#include <server/nautical/WGS84.h>
#include <server/plot/PlotUtils.h>
#include <server/plot/CairoUtils.h>
#include <server/math/Curve2dFilter.h>
#include <server/common/ArrayIO.h>
#include <server/common/indexed.h>

namespace sail {

template <typename T>
using Vec2 = sail::Curve2dFilter::Vec2<T>;

typedef Vec2<Length<double>> Position2d;
typedef Vec2<Velocity<double>> Motion2d;

Curve2dFilter::Settings makeDefaultOptSettings() {
  Curve2dFilter::Settings s;
  s.regWeights = Array<double>{100, 10};
  return s;
}

namespace {

  template <typename Range>
  Array<TimeStamp> getTimeStamps(const Range &r) {
    ArrayBuilder<TimeStamp> dst(r.end() - r.begin());
    for (auto x: r) {
      dst.add(x.time);
    }
    return dst.get();
  }

  Array<TimedValue<Vec2<Length<double>>>> getLocalPositions(
      const GeographicReference &geoRef,
      const Array<TimedValue<GeographicPosition<double> >> &positions) {
    int n = positions.size();
    Array<TimedValue<Vec2<Length<double>>>> dst(n);
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

Array<TimedValue<Vec2<Velocity<double>>>> toLocalMotions(
    const Array<TimedValue<HorizontalMotion<double>>> &motions) {
  int n = motions.size();
  Array<TimedValue<Vec2<Velocity<double>>>> dst(n);
  for (int i = 0; i < n; i++) {
    const auto& m = motions[i];
    dst[i] = TimedValue<Vec2<Velocity<double>>>(
        m.time, Vec2<Velocity<double>>(m.value));
  }
  return dst;
}


Array<TimedValue<Position2d>> removePositionsFarAway(
    const Array<TimedValue<Position2d>> &src,
    Length<double> maxLen) {
  ArrayBuilder<TimedValue<Position2d>> dst(src.size());
  for (auto x: src) {
    auto p = x.value;
    if (sqr(double(p[0]/maxLen)) + sqr(double(p[1]/maxLen)) < 1.0) {
      dst.add(x);
    }
  }
  return dst.get();
}

Array<TimedValue<Position2d>> to2dPositions(
    const GeographicReference& ref,
    const Array<TimedValue<GeographicPosition<double>>>& src) {
  using namespace Curve2dFilter;
  int n = src.size();
  Array<TimedValue<Position2d>> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = TimedValue<Position2d>(src[i].time, ref.map(src[i].value));
  }
  return dst;
}

TimedValue<Motion2d> toMotion2d(const TimedValue<HorizontalMotion<double>>& x) {
  return TimedValue<Motion2d>(x.time, Motion2d{x.value[0], x.value[1]});
}

Array<TimedValue<Motion2d>> to2dMotions(
    const Array<TimedValue<HorizontalMotion<double>>>& src) {
  return sail::map(src, &toMotion2d);
}

Array<TimedValue<GeographicPosition<double>>>
  LocalGpsFilterResults::samplePositions() const {
  ArrayBuilder<TimedValue<GeographicPosition<double>>> dst(
      filterResults.timeMapper.sampleCount());
  for (auto curve: curves) {
    const auto& m = curve.timeMapper();
    auto span = curve.indexSpan();
    for (int i: span) {
      auto t = m.toTimeStamp(i);
      auto pos = Position2d{
        curve.evaluate(0, t),
        curve.evaluate(1, t)
      };
      dst.add(TimedValue<GeographicPosition<double>>(t, geoRef.unmap(pos)));
    }
  }
  return dst.get();
}

Array<TimedValue<HorizontalMotion<double>>>
  LocalGpsFilterResults::sampleMotions() const {
  ArrayBuilder<TimedValue<HorizontalMotion<double>>> dst(
      filterResults.timeMapper.sampleCount());
  for (auto curve: curves) {
    const auto& m = curve.timeMapper();
    auto c = curve.derivative();
    auto span = curve.indexSpan();
    for (int i: span) {
      auto t = m.toTimeStamp(i);
      HorizontalMotion<double> x{
        c.evaluate(0, t), c.evaluate(1, t)
      };
      dst.add(TimedValue<HorizontalMotion<double>>(t, x));
    }
  }
  return dst.get();
}

Array<TimedValue<Length<double>>> getGaps(
    const Array<TimedValue<Vec2<Length<double>>>>& positions) {
  int n = std::max(0, positions.size() - 1);
  ArrayBuilder<TimedValue<Length<double>>> dst(n);
  for (int i = 0; i < n; i++) {
    const auto& a = positions[i];
    const auto& b = positions[i+1];
    auto t = a.time + 0.5*(b.time - a.time);
    auto d = (b.value - a.value).norm();
    dst.add(TimedValue<Length<double>>(t, d));
  }
  return dst.get();
}

void populate(const Array<TimedValue<Length<double>>>& lengths,
    int at, Array<Length<double>>* buf) {
  int w = buf->size();
  int w2 = w/2;
  int from = at - w2;
  int n = lengths.size();
  for (int i = 0; i < w; i++) {
    auto k = mirror(from + i, n-1);
    CHECK(0 <= k);
    CHECK(k < lengths.size());
    (*buf)[i] = lengths[k].value;
  }
}

Array<TimedValue<Length<double>>> localMedianLengths(
    const Array<TimedValue<Length<double>>>& lengths, int w) {
  if (lengths.empty()) {
    return Array<TimedValue<Length<double>>>();
  }
  Array<Length<double>> buf(w);
  int n = lengths.size();
  ArrayBuilder<TimedValue<Length<double>>> result(n);
  for (int i = 0; i < n; i++) {
    populate(lengths, i, &buf);
    std::sort(buf.begin(), buf.end());
    result.add(TimedValue<Length<double>>(
        lengths[i].time,
        buf[buf.middle()]));
  }
  return result.get();
}



Array<Eigen::Vector2d> getGapPoints(
    const Array<TimedValue<Length<double>>>& gaps) {
  int n = gaps.size();
  ArrayBuilder<Eigen::Vector2d> dst(n);
  for (auto kv: indexed(gaps)) {
    dst.add(Eigen::Vector2d(kv.first, kv.second.value.meters()));
  }
  return dst.get();
}

void visualizeRawLocalGaps(
    const Array<TimedValue<Length<double>>>& gaps,
    const Array<TimedValue<Length<double>>>& filteredGaps,
    DOM::Node* dst) {
  if (dst->defined()) {
    auto subpage = DOM::linkToSubPage(dst, "Gap plot");
    Poco::Path p = DOM::makeGeneratedImageNode(&subpage, ".svg");
    auto setup = Cairo::Setup::svg(p.toString(), 800.0, 600.0);
    PlotUtils::Settings2d settings;
    settings.orthonormal = false;
    Cairo::renderPlot(settings, [&](cairo_t* cr) {
      Cairo::plotLineStrip(cr, getGapPoints(gaps)); //, 3);
      Cairo::setSourceColor(cr, PlotUtils::HSV::fromHue(215.0_deg));
      Cairo::plotLineStrip(cr, getGapPoints(filteredGaps)); //, 3);
    }, "Index", "Gap (meters)", setup.cr.get());
    std::cout << "Done rendering" << std::endl;
  }
}

struct LargeGap {
  Length<double> distanceToSupport;
  Length<double> threshold;
};

Array<LocalGpsFilterResults::Curve> segmentCurvesByDistanceThreshold(
    const LocalGpsFilterResults::Curve& curve,
    Length<double> inlierThreshold,
    const Array<TimedValue<Vec2<Length<double>>>>& inlierPositions,
    int medianWindowLength,
    DOM::Node* out) {
  auto gaps = getGaps(inlierPositions);
  auto filtered = localMedianLengths(gaps, medianWindowLength);
  visualizeRawLocalGaps(gaps, filtered, out);
  int n = gaps.size();
  ArrayBuilder<TimedValue<bool>> goodBuilder(2*n);
  std::vector<LargeGap> largeGaps;
  for (int i = 0; i < n; i++) {
    const auto& y = filtered[i];
    auto pos = Vec2<Length<double>>{
      curve.evaluate(0, y.time),
      curve.evaluate(1, y.time)
    };
    const auto& a = inlierPositions[i];
    goodBuilder.add(TimedValue<bool>(a.time, true));
    auto maxl = std::max(
        (pos - a.value).norm(),
        (pos - inlierPositions[i+1].value).norm());
    auto threshold = y.value + inlierThreshold;
    if (threshold < maxl) {
      goodBuilder.add(TimedValue<bool>(y.time, false));
      largeGaps.push_back(LargeGap{maxl, threshold});
    }
  }
  goodBuilder.add(TimedValue<bool>(filtered.last().time, true));
  auto good = goodBuilder.get();
  CHECK(std::is_sorted(good.begin(), good.end()));

  Duration<double> margin = 1.0_minutes;
  auto segments = SampleUtils::makeGoodSpans(good,
      margin, margin);

  if (segments.size() == 1 && !largeGaps.empty()) {
    DOM::addSubTextNode(out, "p", "This is really strange").error();
    auto name = out->writer->generatePath("goodmask.dat").toString();
    saveRawArray<TimedValue<bool>>(name, good);
    DOM::addSubTextNode(out, "p", "Save it to " + name);
  }

  int segmentCount = segments.size();
  ArrayBuilder<LocalGpsFilterResults::Curve> resultsBuilder(segmentCount);
  for (auto s: segments) {
    auto c = curve.slice(s);
    if (!c.empty()) {
      resultsBuilder.add(c);
    }
  }
  auto results = resultsBuilder.get();

  if (2 <= results.size()) {
    DOM::addSubTextNode(out, "p",
        stringFormat(
            "Large distance gap resulted in "
            "the curve being cut in %d pieces", results.size()))
    .warning();
  } else {
    DOM::addSubTextNode(out, "p", "Curve was not cut").success();
  }
  if (2 <= segments.size()) {
    DOM::addSubTextNode(out, "p",
        stringFormat("Number of segments detected: %d", segments.size()));
    auto list = DOM::makeSubNode(out, "ul");
    for (auto s: segments) {
      DOM::addSubTextNode(&list, "li", stringFormat(
          "Segment form %s to %s", s.minv().toIso8601String().c_str(),
          s.maxv().toIso8601String().c_str()));
    }

  }
  if (!largeGaps.empty()) {
    DOM::addSubTextNode(out, "p", "Large gaps").warning();
    auto list = DOM::makeSubNode(out, "ul");
    for (auto lg: largeGaps) {
      DOM::addSubTextNode(&list,
          "li",
          stringFormat(
              "Distance to closest support %.3g meters "
              "and threshold %.3g meters", lg.distanceToSupport.meters(),
              lg.threshold.meters()
          ));
    }
  }


  return results;
}

LocalGpsFilterResults solveGpsSubproblem(
    const TimeMapper& mapper,
    const Array<TimedValue<GeographicPosition<double>>> rawPositions,
    const Array<TimedValue<HorizontalMotion<double>>> &motions,
    const GpsFilterSettings &settings,
    DOM::Node *dst) {
  if (mapper.lastSampleTime() < TimeStamp::UTC(1990, 1, 1, 1, 1, 1)) {
    DOM::addSubTextNode(dst, "p",
        stringFormat("This data with last sample at %s "
            "is ancient, ignore it", mapper.lastSampleTime()
            .toIso8601String().c_str())).warning();
    return LocalGpsFilterResults();
  }

  auto referencePosition = GpsUtils::getReferencePosition(rawPositions);
  GeographicReference geoRef(referencePosition);

  auto rawLocalPositions = getLocalPositions(geoRef, rawPositions);


  TimeStamp start = TimeStamp::now();
  auto positionsForFilter = to2dPositions(geoRef, rawPositions);
  auto motionsForFilter = to2dMotions(motions);
  LOG(INFO) << "Optimizing sub problem on " << mapper.sampleCount() << " samples...";
  Curve2dFilter::Results results = Curve2dFilter::optimize(mapper,
      positionsForFilter, motionsForFilter,
      settings.curveFilterSettings);
  if (!results.OK()) {
    return LocalGpsFilterResults();
  }
  auto curves = segmentCurvesByDistanceThreshold(
      results.curve(), settings.positionSupportThreshold,
      results.inlierPositions,
      settings.medianWindowLength,
      dst);

  CHECK(rawPositions.size() == positionsForFilter.size());

  for (auto curve: curves) {
    auto motion = curve.derivative();
    auto acceleration = motion.derivative();
    auto maxSpeed = computeMaxNorm<2>(motion);
    if (maxSpeed.value > 50.0_kn && bool(dst->writer)) {
      auto toffs = curve.timeMapper().offset();
      auto pref = dst->writer->generatePath("problem_data").toString();
      auto posName = pref + "_positions.dat";
      auto motName = pref + "_motions.dat";
      auto timeName = pref + "_timemapper.dat";
      DOM::addSubTextNode(dst, "p", stringFormat(
          "Unusually high filtered boat speed of %.3g knots at %s "
          "in session starting at %s after %s. "
          "Source positions, motions and mapper serialized "
          "to %s, %s and %s, respectively.",
          maxSpeed.value.knots(),
          maxSpeed.time.toIso8601String().c_str(),
          toffs.toIso8601String().c_str(),
          (maxSpeed.time - toffs).str().c_str(),
          posName.c_str(), motName.c_str(), timeName.c_str()))
        .warning();

      auto maxAcc = computeMaxNorm<2>(acceleration);
      DOM::addSubTextNode(dst, "p", stringFormat(
          "The max acceleration is %.3g m/s^2 after %s",
          double(maxAcc.value/(1.0_mps/1.0_s)),
          (maxAcc.time - toffs).str().c_str()));

      saveRawArray<TimedValue<GeographicPosition<double>>>(
                posName, rawPositions);
      saveRawArray<TimedValue<HorizontalMotion<double>>>(
          motName, motions);
      saveRawArray<TimeMapper>(timeName, {mapper});
    }
  }



  return LocalGpsFilterResults{
    geoRef, results,
    TimeStamp::now() - start,
    rawLocalPositions,
    curves
  };
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

Velocity<double> getMaxSpeed(
    const TimedSampleCollection<HorizontalMotion<double> >::TimedVector &src) {
  auto m = 0.0_kn;
  for (auto x: src) {
    m = std::max(m, x.value.norm());
  }
  return m;
}

Length<double> getMaxPositionGap(
    const TimedSampleCollection<GeographicPosition<double> >::TimedVector &src) {
  auto m = 0.0_m;
  for (int i = 0; i < src.size()-1; i++) {
    m = std::max(m, sail::distance(src[i].value, src[i+1].value));
  }
  return m;
}


auto plotUnit = 1.0_km;

Eigen::Vector2d tov2(
    const TimedValue<Position2d> &x) {
  return Eigen::Vector2d(
      x.value[0]/plotUnit,
      x.value[1]/plotUnit);
}

Array<Eigen::Vector2d> toV2(
    const Array<TimedValue<Position2d>> &src) {
  return map(src, &tov2);
}

Array<Eigen::Vector2d> toV2(
    const LocalGpsFilterResults::Curve& curve) {
  auto m = curve.timeMapper();
  auto span = curve.indexSpan();
  ArrayBuilder<Eigen::Vector2d> pts(span.size());
  for (auto i: curve.indexSpan()) {
    auto t = m.toTimeStamp(i);
    pts.add(Eigen::Vector2d(
        curve.evaluate(0, t)/plotUnit,
        curve.evaluate(1, t)/plotUnit));
  }
  return pts.get();
}

void outputLocalResults(
    const LocalGpsFilterResults& r,
    DOM::Node *dst) {
  auto page = DOM::linkToSubPage(dst, "Trajectory");
  auto imageFilename = DOM::makeGeneratedImageNode(
      &page, ".svg").toString();

  auto setup = Cairo::Setup::svg(imageFilename, 800, 600);


  PlotUtils::Settings2d settings;
  Cairo::renderPlot(settings, [&](cairo_t *cr) {
    Cairo::plotDots(cr, toV2(r.rawLocalPositions), 0.5);
    Cairo::setSourceColor(cr, PlotUtils::HSV::fromHue(240.0_deg));
    cairo_set_line_width (cr, 0.5);
    for (auto curve: r.curves) {
      auto pts = toV2(curve);
      Cairo::plotLineStrip(cr, pts);
    }
  }, "X", "Y", setup.cr.get());
}

template <typename T>
Velocity<double> findMaxSpeed(
    const T& src) {
  auto x = 0.0_kn;
  for (auto y: src) {
    x = std::max(x, y.value.norm());
  }
  return x;
}

GpsFilterResults mergeSubResults(
    const std::vector<LocalGpsFilterResults> &subResults,
    Duration<double> thresh,
    DOM::Node *log) {
  DOM::addSubTextNode(log, "h2", "Merge gps sub results");

  if (subResults.empty()) {
    return GpsFilterResults();
  }

  TimedSampleCollection<GeographicPosition<double> >::TimedVector positions;
  TimedSampleCollection<HorizontalMotion<double> >::TimedVector motions;

  Duration<double> totalRealTime = 0.0_s;
  Duration<double> totalComputationTime = 0.0_s;
  int totalSamples = 0;

  for (int i = 0; i < subResults.size(); i++) {
    auto body = DOM::makeSubNode(log, "pre");
    auto x = subResults[i];
    auto pos = x.samplePositions();
    auto mot = x.sampleMotions();
    DOM::addLine(&body, stringFormat("  * Sub results %d", i));
    auto m = x.filterResults.timeMapper;
    totalSamples += m.sampleCount();

    auto from = m.firstSampleTime();
    auto to = m.lastSampleTime();

    totalRealTime += std::max(to - from, 0.0_s);
    totalComputationTime = x.computationTime;


    if (x.empty()) {
      DOM::addLine(&body, "    Empty sub result");
    } else {
      DOM::addLine(&body, stringFormat("    From %s to %s",
          from.toString().c_str(),
          to.toString().c_str()));
    }
    DOM::addLine(&body, stringFormat(
        "    %d positions and duration %s",
        m.sampleCount(), (to - from).str().c_str()));
    DOM:addLine(&body, stringFormat(
        "    Max speed: %.3g knots",
        findMaxSpeed(mot).knots()));

    if (body.defined()) {
      outputLocalResults(x, &body);
    }

    for (auto y: pos) {
      positions.push_back(y);
    }
    for (auto y: mot) {
      motions.push_back(y);
    }


    DOM::addLine(&body, "");
  }
  auto m2 = findMaxSpeed(motions);
  auto speedNode = DOM::addSubTextNode(log, "p",
      stringFormat("Max speed overall: %.3g knots",
          m2.knots()));

  DOM::addSubTextNode(log, "p",
      stringFormat("Total number of samples: %d", totalSamples));
  DOM::addSubTextNode(log, "p",
      stringFormat("Total computation time: %s",
          totalComputationTime.str().c_str()));
  DOM::addSubTextNode(log, "p",
      stringFormat("Total real time: %s",
          totalRealTime.str().c_str()));
  DOM::addSubTextNode(log, "p",
      stringFormat("Computation time per sample: %.3g",
          (1.0/totalSamples)*totalComputationTime));
  DOM::addSubTextNode(log, "p",
      stringFormat("Computation/Real time: %.3g",
          double(totalComputationTime/totalRealTime)));

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

bool isSingleOutlier(
    const TimedValue<GeographicPosition<double>> &a,
    const TimedValue<GeographicPosition<double>> &b,   //<-- This is the one we are considering
    const TimedValue<GeographicPosition<double>> &c) {
  auto ab = sail::distance(a.value, b.value);
  auto ac = sail::distance(a.value, c.value);
  auto bc = sail::distance(b.value, c.value);

  // The middle point is off-track, but the neighbours
  // on either side are close to each other.
  return ac < distanceSplitThreshold && std::max(ab, bc) > distanceSplitThreshold;
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

TimeStamp getCuttingTime(
    const Array<TimedValue<GeographicPosition<double>>>& src,
    int index) {
  TimeStamp a = src[index-1].time;
  TimeStamp b = src[index].time;
  return a + 0.5*(b - a);
}

Array<Span<TimeStamp>> makeBadSpans(
    const Array<TimedValue<GeographicPosition<double>>>& src,
    const std::vector<int>& inds) {
  ArrayBuilder<Span<TimeStamp>> dst;
  if (inds.empty()) {
    return dst.get();
  }
  int from = inds.front();
  int current = from;
  for (int i = 1; i < inds.size(); i++) {
    int next = inds[i];
    if (current + 1 == next) {
      current = next;
    } else {
      dst.add(Span<TimeStamp>(
          getCuttingTime(src, from),
          getCuttingTime(src, current)));
      from = next;
      current = next;
    }
  }
  return dst.get();
}

PositionPrefiltering removeSingleOutliers(
    const Array<TimedValue<GeographicPosition<double>>> &src,
    DOM::Node *log) {

  DOM::addSubTextNode(log, "h4", "Single outlier removal");

  if (src.size() < 3) {
    return src.empty()? PositionPrefiltering() :
        PositionPrefiltering{{}, {
            Span<TimeStamp>(src.first().time,
                src.last().time)
        }};
  }

  ArrayBuilder<TimedValue<GeographicPosition<double>>> dst(src.size());
  int n = src.size();
  std::vector<int> badIndices;
  for (int i = 1; i < n-1; i++) {
    auto x = src[i];
    if (!isSingleOutlier(src[i-1], x, src[i+1])) {
      dst.add(x);
    } else {
      badIndices.push_back(i);
    }
  }
  auto results = dst.get();
  DOM::addSubTextNode(log, "p", stringFormat("Keep %d of %d points",
      results.size(), src.size()));
  return PositionPrefiltering{
    results, makeBadSpans(src, badIndices)
  };
}

PositionPrefiltering prefilterPositions(
    const Array<TimedValue<GeographicPosition<double>>> &src0,
    DOM::Node *log) {

  if (src0.empty()) {
    DOM::addSubTextNode(log, "h3", "No data to prefilter")
      .setAttribute("class", "warning");
  }

  DOM::addSubTextNode(log, "h3", "Filter from "
      + src0.first().time.toString()
      + " to " + src0.last().time.toString());

  return removeSingleOutliers(src0, log);
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
  auto body = DOM::makeSubNode(log, "pre");
  if (src.empty()) {
    DOM::addLine(&body, "No GPS motions");
    return src;
  }
  DOM::addLine(&body,
      stringFormat("From %s to %s",
          src.first().time.toString().c_str(),
          src.last().time.toString().c_str()));


  DOM::addLine(&body, "Unreliable spans");
  for (auto sp: unreliableSpans) {
    DOM::addLine(&body,
        "  * " + sp.minv().toString()
        + " to " + sp.maxv().toString());
  }
  DOM::addLine(&body,
      stringFormat(" in total %d", unreliableSpans.size()));

  ArrayBuilder<TimedValue<HorizontalMotion<double>>> dst(src.size());
  auto maxv = 0.0_kn;
  for (auto x: src) {
    if (!isCovered(x.time, unreliableSpans)) {
      maxv = std::max(maxv, x.value.norm());
      dst.add(x);
    }
  }
  DOM::addLine(&body, stringFormat("Max speed: %.3g knots", maxv.knots()));
  auto result = dst.get();
  if (result.size() < src.size()) {
    DOM::addLine(&body,
        stringFormat("Kept %d of %d samples", result.size(), src.size()));
  } else {
    DOM::addLine(&body,
        stringFormat("Kept %d all samples", result.size()));
  }
  return result;
}

struct GpsData {
  Array<TimedValue<GeographicPosition<double>>> positions;
  Array<TimedValue<HorizontalMotion<double>>> motions;
};

Array<TimeStamp> findDistanceSplits(
    const Array<TimedValue<GeographicPosition<double>>> &src) {
  if (src.size() < 2) {
    return Array<TimeStamp>();
  }
  int n = src.size() - 1;
  ArrayBuilder<TimeStamp> dst;
  for (int i = 0; i < n; i++) {
    const auto &a = src[i];
    const auto &b = src[i+1];
    if (shouldSplit(a, b)) {
      dst.add(a.time + 0.5*(b.time - a.time));
    }
  }
  return dst.get();
}

Array<Span<TimeStamp>> getSpans(const Array<Array<TimeStamp>>& src) {
  int n = src.size();
  ArrayBuilder<Span<TimeStamp>> dst(n);
  for (auto x: src) {
    if (!src.empty()) {
      dst.add(Span<TimeStamp>(x.first(), x.last()));
    }
  }
  return dst.get();
}

struct TimeSeg {
  Array<TimeStamp> splits;
  Array<TimeStamp> times;
  Array<Span<TimeStamp>> spans;
};

TimeSeg segmentTime(
    const GpsData &data,
    const GpsFilterSettings &settings,
    bool withDistance) {
  auto positionTimes = getTimeStamps(data.positions);

  auto splits = listSplittingTimeStampsNotTooLong(positionTimes,
      settings.subProblemThreshold, settings.subProblemLength);
  CHECK(std::is_sorted(splits.begin(), splits.end()));
  if (withDistance) {
    splits = concat(Array<Array<TimeStamp>>{splits, findDistanceSplits(data.positions)});
    std::sort(splits.begin(), splits.end());
  }
  return TimeSeg{splits, positionTimes,
    getSpans(applySplits(positionTimes, splits))
  };
}

Array<TimedValue<GeographicPosition<double>>> getPositions(
    const NavDataset &src) {
  auto src0 = src.samples<GPS_POS>();
  int n = src0.size();
  Array<TimedValue<GeographicPosition<double>>> dst(n);
  std::copy(src0.begin(), src0.end(), dst.begin());
  return dst;
}

GpsData getGpsData(const NavDataset &src) {
  return GpsData{
    getPositions(src),
    GpsUtils::getGpsMotions(src)
  };
}


GpsData prefilterAllData(
    const GpsData &data,
    const GpsFilterSettings &settings,
    DOM::Node *log) {
  auto pp = prefilterPositions(data.positions, log);
  auto pm = maskUnreliable(pp.unreliableSpans, data.motions, log);
  return GpsData{pp.goodPositions, pm};
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

  auto rawData = getGpsData(ds);
  if (rawData.positions.empty()) {
    LOG(ERROR) << "No GPS positions in dataset, cannot filter";
    return GpsFilterResults();
  }

  auto cleanData = prefilterAllData(rawData, settings, log);

  auto time = segmentTime(cleanData, settings, false);
  auto positionSlices = applySplits(cleanData.positions, time.splits);
  auto motionSlices = applySplits(cleanData.motions, time.splits);

  std::vector<LocalGpsFilterResults> subResults;
  subResults.reserve(time.spans.size());

  DOM::addSubTextNode(log, "h2", "Producing GPS filter sub results");
  auto ol = DOM::makeSubNode(log, "ol");


  int n = time.spans.size();
  int last = n-1;
  for (int i = 0; i < n; i++) {
    LOG(INFO) << "Running GPS filter for span "
        << i+1 << "/" << time.spans.size();
    auto positionSlice = positionSlices[i];
    auto motionSlice = motionSlices[i];
    auto li = DOM::makeSubNode(&ol, "li");

    auto span = time.spans[i];
    auto from = span.minv() - 0.5_s;
    auto to = span.maxv() + 0.5_s;
    DOM::addSubTextNode(&li, "p",
        stringFormat("Input:  %d positions, %d motions, over a span of %s",
        positionSlice.size(), motionSlice.size(), (to - from).str().c_str()));

    int sampleCount = int(ceil((to - from)/settings.samplingPeriod));
    TimeMapper mapper(from, settings.samplingPeriod,
        sampleCount);

    if (4 <= sampleCount && !positionSlice.empty()) {
       auto subResult = solveGpsSubproblem(
           mapper, positionSlice,
           motionSlice, settings, &li);
       if (!subResult.empty()) {
         std::stringstream msg;
         msg << "Optimized " << subResult
             .filterResults.timeMapper.sampleCount()
             << " samples in " << subResult.computationTime.str();
         LOG(INFO) << msg.str();
         DOM::addSubTextNode(&li, "p", msg.str());
         subResults.push_back(subResult);
       } else {
         std::stringstream msg;
         msg << "Failed to optimize";
         LOG(ERROR) << msg.str();
         DOM::addSubTextNode(&li, "p", msg.str()).warning();
       }
    } else {
      DOM::addSubTextNode(&li, "p", "Too few positions or times")
        .warning();
    }
    if (i < last) {
      DOM::addSubTextNode(&li, "p",
          stringFormat("Gap to next session: %s",
              (time.spans[i+1].minv() - span.maxv()).str().c_str()));
    }
  }
  return mergeSubResults(subResults,
      settings.subProblemThreshold, log);
}

}
