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

namespace sail {

template <DataCode code>
Array<std::string> listSourcesOfType(
    const Array<CalibDataChunk> &chunks) {
  ArrayBuilder<std::string> dst;
  for (auto chunk: chunks) {
    auto *x = ChannelFieldAccess<code>::get(chunk);
    for (auto kv: *x) {
      dst.add(kv.first);
    }
  }
  return dst.get();
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
    const Array<TimedValue<Angle<double>>> &headings) {
  using namespace Cairo;


  PlotUtils::Settings2d settings;
  settings.width = 400;
  settings.height = 300;
  auto surface = sharedPtrWrap(
      cairo_svg_surface_create(
          filename.c_str(),
          settings.width, settings.height));
  auto cr = sharedPtrWrap(cairo_create(surface.get()));

  auto traj = headingsToTrajectory(headings);
  if (!traj.empty()) {
    auto box = computeBBox(traj);
    auto pose = PlotUtils::computeTotalProjection(box, settings);
    auto cm = Cairo::toCairo(pose);
    cairo_set_matrix(cr.get(), &cm);
    auto f = traj.first().value;
    cairo_move_to(cr.get(), f(0), f(1));
    for (auto pt: traj) {
      auto x = pt.value;
      cairo_line_to(cr.get(), x(0), x(1));
    }
    Cairo::WithLocalDeviceScale wlc(cr.get(),
        Cairo::WithLocalDeviceScale::Identity);
    cairo_stroke(cr.get());
  }
}

void outputMagHeadingPlot(
    const Array<TimedValue<Angle<double>>> &headings,
    DOM::Node *dst) {
  Duration<double> dur = 0.0_s;
  if (!headings.empty()) {
    dur = headings.last().time - headings.first().time;
  }
  DOM::addSubTextNode(dst, "p", stringFormat(
      "Duration of %s", dur.str().c_str()));
  auto imgFilename = DOM::makeGeneratedImageNode(dst, ".svg");
  outputMagHeadingPlotToFile(imgFilename.toString(),
      headings);
}

void outputMagHeadingPlotPerChunk(
    const std::string &src,
    const Array<CalibDataChunk> &chunks,
    DOM::Node *tr) {
  for (auto chunk: chunks) {
    auto td = DOM::makeSubNode(tr, "td");
    auto f = chunk.MAG_HEADING.find(src);
    if (f != chunk.MAG_HEADING.end()) {
      outputMagHeadingPlot(f->second, &td);
    }
  }
}

void outputMagHeadingPlots(
    const Array<std::string> &sources,
    const Array<CalibDataChunk> &chunks,
    DOM::Node *dst) {
  auto table = DOM::makeSubNode(dst, "table");
  for (auto src: sources) {
    auto row = DOM::makeSubNode(&table, "tr");
    DOM::addSubTextNode(&row, "th", src);
    outputMagHeadingPlotPerChunk(src, chunks, &row);
  }
}

bool precalibrateMagHdg(
    const Array<CalibDataChunk> &chunks,
    const ReconstructionSettings &settings,
    DOM::Node *dst) {
  auto sources = listSourcesOfType<MAG_HEADING>(chunks);
  outputMagHeadingPlots(sources, chunks, dst);

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
