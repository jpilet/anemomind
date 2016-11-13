/*
 * Reconstructor.cpp
 *
 *  Created on: 16 Oct 2016
 *      Author: jonas
 */

#include <server/nautical/calib/Reconstructor.h>
#include <server/plot/AxisTicks.h>
#include <server/plot/PlotUtils.h>
#include <server/plot/CairoUtils.h>
#include <server/common/ArrayBuilder.h>
#include <cairo/cairo-svg.h>
#include <server/common/string.h>
#include <server/plot/ColorMap.h>
#include <server/nautical/GeographicReference.h>
#include <server/common/indexed.h>
#include <server/nautical/calib/MagHdgCalib.h>
#include <server/nautical/calib/MagHdgCalib2.h>

namespace sail {

template <DataCode code>
std::set<std::string> listSourcesOfType(
    const Array<CalibDataChunk> &chunks) {
  std::set<std::string> dst;
  for (auto chunk: chunks) {
    auto *x = ChannelFieldAccess<code>::get(chunk);
    for (auto kv: *x) {
      dst.insert(kv.first);
    }
  }
  return dst;
}

Array<TimedValue<Eigen::Vector2d>> headingsToTrajectory(
    const Array<TimedValue<Angle<double>>> &headings) {
  if (headings.empty()) {
    return Array<TimedValue<Eigen::Vector2d>>();
  }
  auto f = headings.first();
  ArrayBuilder<TimedValue<Eigen::Vector2d>> dst;
  auto last = TimedValue<Eigen::Vector2d>(
      f.time, Eigen::Vector2d(0.0, 0.0));
  dst.add(last);
  for (auto h: headings) {
    auto dur = (h.time - last.time).seconds();
    auto m = HorizontalMotion<double>::polar(1.0_mps, h.value);
    Eigen::Vector2d nextPos = last.value +
        dur*Eigen::Vector2d(
            m[0].metersPerSecond(),
            m[1].metersPerSecond());
    last = TimedValue<Eigen::Vector2d>(h.time, nextPos);
    dst.add(last);
  }
  return dst.get();
}

Array<TimedValue<Eigen::Vector2d>> gpsPosToTrajectory(
    const Array<TimedValue<GeographicPosition<double>>> &pos) {
  if (pos.empty()) {
    return Array<TimedValue<Eigen::Vector2d>>();
  }
  auto middle = pos[pos.size()/2];
  GeographicReference ref(middle.value);
  int n = pos.size();
  Array<TimedValue<Eigen::Vector2d>> dst(n);
  for (int i = 0; i < n; i++) {
    auto x = pos[i];
    auto y = ref.map(x.value);
    dst[i] = TimedValue<Eigen::Vector2d>(
        x.time, Eigen::Vector2d(y[0].meters(), y[1].meters()));
  }
  return dst;
}

template <DataCode code>
using ToTrajectory = std::function<Array<TimedValue<Eigen::Vector2d>>(
        Array<TimedValue<GetTypeForCode<code>>>)>;

void outputMagHeadingPlotToFile(
    const std::string &filename,
    const std::string &unit,
    const Array<TimedValue<Eigen::Vector2d>> &traj) {
  using namespace Cairo;


  PlotUtils::Settings2d settings;
  settings.width = 400;
  settings.height = 300;
  auto surface = sharedPtrWrap(
      cairo_svg_surface_create(
          filename.c_str(),
          settings.width, settings.height));
  auto cr = sharedPtrWrap(cairo_create(surface.get()));

  if (!traj.empty()) {
    Cairo::renderPlot(settings, [&](cairo_t *cr) {
      auto cmap = makeTimeColorMap(
          traj.first().time,
          traj.last().time);
      auto f = traj.first().value;
      cairo_move_to(cr, f(0), f(1));
      auto last = traj.first();
      for (int i = 0; i < traj.size(); i++) {
        auto perform = i % 30 == 0 || i == traj.size()-1;
        auto pt = traj[i];
        auto x = pt.value;
        Cairo::setSourceColor(cr, cmap(pt.time));
        if (perform) {
          cairo_move_to(cr, last.value(0), last.value(1));
        }
        cairo_line_to(cr, x(0), x(1));
        if (perform) {
          {
            Cairo::WithLocalDeviceScale wlc(cr,
                Cairo::WithLocalDeviceScale::Identity);
            cairo_stroke(cr);
          }
          cairo_move_to(cr, x(0), x(1));
        }
        last = pt;
      }
    }, unit, unit, cr.get());
  }
}

void outputTrajectoryDuration(
    const Array<TimedValue<Eigen::Vector2d>> &traj,
    DOM::Node *dst) {
  Duration<double> dur = 0.0_s;
  if (!traj.empty()) {
    dur = traj.last().time - traj.first().time;
  }
  DOM::addSubTextNode(dst, "p", stringFormat(
      "Duration of %s", dur.str().c_str()));
}

void outputMagHeadingPlot(
    const std::string &unit,
    const Array<TimedValue<Eigen::Vector2d>> &traj,
    DOM::Node *dst) {
  outputTrajectoryDuration(traj, dst);
  auto imgFilename = DOM::makeGeneratedImageNode(dst, ".svg");
  outputMagHeadingPlotToFile(imgFilename.toString(), unit, traj);
}

template <DataCode code>
void outputMagHeadingPlotPerChunk(
    const std::string &src,
    std::string unit,
    const Array<CalibDataChunk> &chunks,
    ToTrajectory<code> toTraj,
    DOM::Node *tr) {
  for (auto chunk: chunks) {
    auto td = DOM::makeSubNode(tr, "td");
    const auto *m = ChannelFieldAccess<code>::get(chunk);
    auto f = m->find(src);
    if (f != m->end()) {
      outputMagHeadingPlot(
          unit, toTraj(f->second), &td);
    }
  }
}

template <DataCode code>
void outputTrajectoryPlots(
    const Array<CalibDataChunk> &chunks,
    std::string unit,
    ToTrajectory<code> f,
    DOM::Node *dst) {
  auto sources = listSourcesOfType<code>(chunks);
  {
    auto ul = DOM::makeSubNode(dst, "ul");
    for (auto src: sources) {
      DOM::addSubTextNode(&ul, "li", src);
    }
  }
  auto table = DOM::makeSubNode(dst, "table");
  for (auto src: sources) {
    auto row = DOM::makeSubNode(&table, "tr");
    DOM::addSubTextNode(&row, "th", src);
    outputMagHeadingPlotPerChunk<code>(src, unit, chunks, f, &row);
  }
}

Array<TimedValue<Eigen::Vector2d>> sample2dPositions(
    const SplineGpsFilter::EcefCurve &curve) {
  CHECK(curve.defined());
  CHECK(curve.lower().defined());
  CHECK(curve.upper().defined());
  auto middleTime = curve.lower() + 0.5*(curve.upper() - curve.lower());
  auto middlePos = curve.evaluateGeographicPosition(middleTime);
  GeographicReference geoRef(middlePos);
  auto timeMapper = curve.timeMapper();
  Array<TimedValue<Eigen::Vector2d>> dst(timeMapper.sampleCount);
  for (int i = 0; i < timeMapper.sampleCount; i++) {
    auto t = timeMapper.unmap(i);
    auto xy = geoRef.map(curve.evaluateGeographicPosition(t));
    dst[i] = TimedValue<Eigen::Vector2d>(t,
        Eigen::Vector2d(xy[0].meters(), xy[1].meters()));
  }
  return dst;
}

void outputFilteredPositionsPlots(
    const Array<CalibDataChunk> &chunks,
    DOM::Node *dst) {
  auto table = DOM::makeSubNode(dst, "table");
  auto row = DOM::makeSubNode(&table, "tr");
  DOM::addSubTextNode(&row, "th", "Filtered positions");
  for (auto chunk: chunks) {
    auto td = DOM::makeSubNode(&row, "td");
    auto traj = sample2dPositions(chunk.trajectory);
    outputMagHeadingPlot("meters", traj, &td);
  }
}

bool precalibrateMagHdg(
    const Array<CalibDataChunk> &chunks,
    const ReconstructionSettings &settings,
    DOM::Node *dst) {
  return true;
}

bool areValidChunks(const Array<CalibDataChunk> &chunks) {
  for (auto c: chunks) {
    if (!c.trajectory.defined()) {
      return false;
    }
  }
  return true;
}

void outputOtherSignals(DOM::Node *dst,
    const Array<CalibDataChunk> &chunks) {
  DOM::addSubTextNode(dst, "h2",
      stringFormat("Calib data chunks: %d", chunks.size()));
  for (auto kv: indexed(chunks)) {
    DOM::addSubTextNode(dst, "h2",
        stringFormat("Calib data chunk %d/%d", kv.first,
            chunks.size()));
  }
}

ReconstructionResults reconstruct(
    const Array<CalibDataChunk> &chunks,
    const ReconstructionSettings &settings,
    DOM::Node *dst) {
  CHECK(areValidChunks(chunks));
  DOM::addSubTextNode(dst, "h2", "Magnetic headings");
  outputTrajectoryPlots<MAG_HEADING>(
      chunks, "unit", &headingsToTrajectory, dst);

  DOM::addSubTextNode(dst, "h2", "Raw GPS trajectories");
  outputTrajectoryPlots<GPS_POS>(
      chunks, "meters", &gpsPosToTrajectory, dst);

  DOM::addSubTextNode(dst, "h2", "Filtered GPS trajectories");
  outputFilteredPositionsPlots(chunks, dst);

  DOM::addSubTextNode(dst, "h2", "Magnetic headings");
  for (auto chunk: indexed(chunks)) {
    DOM::addSubTextNode(dst, "h3",
        stringFormat("Chunk %d/%d",
            chunk.first + 1, chunks.size()));
    for (auto signal: chunk.second.MAG_HEADING) {
      DOM::addSubTextNode(dst, "h4",
          stringFormat("For mag heading '%s'",
              signal.first.c_str()));

      if (false) {
        MagHdgCalib::Settings settings;
        MagHdgCalib::calibrateSingleChannel(
            chunk.second.trajectory,
            signal.second, settings, dst);

        MagHdgCalib::makeCostPlot(
            30, chunk.second.trajectory,
            signal.second, dst);
      }
      if (true) {
        Array<MagHdgCalib2::Settings> settings(2);
        MagHdgCalib2::makeAngleFitnessPlot(
            chunk.second.trajectory,
            signal.second, settings,
            dst);
      }
    }
  }

  return ReconstructionResults();
}

}
