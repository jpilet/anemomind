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

BBox3d computeBBox(const Array<TimedValue<Eigen::Vector2d>> &src) {
  BBox3d dst;
  for (auto x : src) {
    auto y = x.value;
    double data[3] = {y(0), y(1), 0.0};
    dst.extend(data);
  }
  return dst;
}

void outputMagHeadingPlotToFile(
    const std::string &filename,
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
    auto box = computeBBox(traj);
    auto pose = PlotUtils::computeTotalProjection(box, settings);
    auto cm = Cairo::toCairo(pose);
    auto cmap = makeTimeColorMap(
        traj.first().time,
        traj.last().time);
    cairo_set_matrix(cr.get(), &cm);
    auto f = traj.first().value;
    cairo_move_to(cr.get(), f(0), f(1));
    auto last = traj.first();
    for (int i = 0; i < traj.size(); i++) {
      auto perform = i % 30 == 0 || i == traj.size()-1;
      auto pt = traj[i];
      auto x = pt.value;
      Cairo::setSourceColor(cr.get(), cmap(pt.time));
      if (perform) {
        cairo_move_to(cr.get(), last.value(0), last.value(1));
      }
      cairo_line_to(cr.get(), x(0), x(1));
      if (perform) {
        {
          Cairo::WithLocalDeviceScale wlc(cr.get(),
              Cairo::WithLocalDeviceScale::Identity);
          cairo_stroke(cr.get());
        }
        cairo_move_to(cr.get(), x(0), x(1));
      }
      last = pt;
    }
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
    const Array<TimedValue<Eigen::Vector2d>> &traj,
    DOM::Node *dst) {
  outputTrajectoryDuration(traj, dst);
  auto imgFilename = DOM::makeGeneratedImageNode(dst, ".svg");
  outputMagHeadingPlotToFile(imgFilename.toString(), traj);
}

template <DataCode code>
void outputMagHeadingPlotPerChunk(
    const std::string &src,
    const Array<CalibDataChunk> &chunks,
    ToTrajectory<code> toTraj,
    DOM::Node *tr) {
  for (auto chunk: chunks) {
    auto td = DOM::makeSubNode(tr, "td");
    const auto *m = ChannelFieldAccess<code>::get(chunk);
    auto f = m->find(src);
    if (f != m->end()) {
      outputMagHeadingPlot(
          toTraj(f->second), &td);
    }
  }
}

template <DataCode code>
void outputTrajectoryPlots(
    const Array<CalibDataChunk> &chunks,
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
    outputMagHeadingPlotPerChunk<code>(src, chunks, f, &row);
  }
}

void outputFilteredPositionsPlots(
    const Array<CalibDataChunk> &chunks,
    DOM::Node *dst) {
  auto table = DOM::makeSubNode(dst, "table");
  auto row = DOM::makeSubNode(&table, "tr");
  DOM::addSubTextNode(&row, "th", "Filtered positions");
  for (auto chunk: chunks) {
    auto td = DOM::makeSubNode(&row, "td");
    auto traj = gpsPosToTrajectory(chunk.filteredPositions);
    outputMagHeadingPlot(traj, &td);
  }
}

bool precalibrateMagHdg(
    const Array<CalibDataChunk> &chunks,
    const ReconstructionSettings &settings,
    DOM::Node *dst) {
  DOM::addSubTextNode(dst, "h2", "Magnetic headings");
  outputTrajectoryPlots<MAG_HEADING>(
      chunks, &headingsToTrajectory, dst);

  DOM::addSubTextNode(dst, "h2", "Raw GPS trajectories");
  outputTrajectoryPlots<GPS_POS>(
      chunks, &gpsPosToTrajectory, dst);
  DOM::addSubTextNode(dst, "h2", "Filtered GPS trajectories");
  outputFilteredPositionsPlots(chunks, dst);
  return true;
}

ReconstructionResults reconstruct(
    const Array<CalibDataChunk> &chunks,
    const ReconstructionSettings &settings,
    DOM::Node *dst) {
  auto magHdgPage = DOM::linkToSubPage(
      dst, "Magnetic heading precalibration");
  auto magHdg = precalibrateMagHdg(chunks, settings, &magHdgPage);

  return ReconstructionResults();
}

}
