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

void outputMagHeadingPlotToFile(
    const std::string &filename,
    const Array<TimedValue<Angle<double>>> &headings) {
  using namespace Cairo;

  double width = 0.0;
  double height = 0.0;
  auto surface = sharedPtrWrap(
      cairo_svg_surface_create(
          filename.c_str(),
          width, height));
  auto cr = sharedPtrWrap(cairo_create(surface.get()));
}

void outputMagHeadingPlot(
    const Array<TimedValue<Angle<double>>> &headings,
    DOM::Node *dst) {
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
