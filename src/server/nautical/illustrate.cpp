/*
 * illustrate.cpp
 *
 *  Created on: 13 Oct 2016
 *      Author: jonas
 */

#include <server/common/ArgMap.h>
#include <server/common/TimeStamp.h>
#include <server/nautical/DownsampleGps.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/BoatLogProcessor.h>
#include <server/nautical/tiles/TileUtils.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/calib/Calibrator.h>
#include <device/anemobox/simulator/SimulateBox.h>
#include <server/common/DOMUtils.h>
#include <server/plot/CairoUtils.h>
#include <server/plot/PlotUtils.h>
#include <cairo/cairo-svg.h>

using namespace sail;

struct Setup {
  std::string path;
  std::string fromStr, toStr;
  std::string prefix = PathBuilder::makeDirectory(
      Env::BINARY_DIR).get().toString();
  std::string name = "illustrate";

  TimeStamp from() const {
    return TimeStamp::parse(fromStr);
  }

  TimeStamp to() const {
    return TimeStamp::parse(toStr);
  }
};

NavDataset loadNavs(const std::string &path) {
  LogLoader loader;
  loader.load(path);
  return loader.makeNavDataset();
}

Array<GeographicReference::ProjectedPosition> positionsToPlane(
    const TimedSampleRange<GeographicPosition<double>> &positions,
    const GeographicReference &ref) {
  int n = positions.size();
  Array<GeographicReference::ProjectedPosition> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = ref.map(positions[i].value);
  }
  return dst;
}



struct BoatRenderSettings {
  PlotUtils::RGB goodWind = PlotUtils::hsv2rgb(PlotUtils::HSV::fromHue(120.0_deg));
  PlotUtils::RGB badWind = PlotUtils::hsv2rgb(PlotUtils::HSV::fromHue(0.0_deg));
  PlotUtils::RGB boat = PlotUtils::hsv2rgb(PlotUtils::HSV::fromHue(215.0_deg));
};

struct RelWind {
  Angle<double> dirWrtBoat;
  Velocity<double> speed;
};

struct BoatData {
  GeographicReference::ProjectedPosition position;
  Angle<double> heading;

  //Optional<Angle<double>> badAwa, goodAwa;
  //Optional<Angle<double>> badTwdir, goodTwdir;
};


void renderBoatData(
    cairo_t *cr,
    const BoatData &boat,
    const BoatRenderSettings &settings) {

}

namespace {
  auto lengthUnit = 1.0_m;
}


void renderBoat(cairo_t *cr,
    const GeographicReference &ref,
    TimeStamp time,
  const NavDataset &ds) {
  auto pos = ds.samples<GPS_POS>().nearest(time);
  if (!pos.defined()) {
    std::cout << "Missing pos";
    return;
  }

  auto localPos = ref.map(pos.get().value);

  cairo_translate(cr,
      localPos[0]/lengthUnit,
      localPos[1]/lengthUnit);


  drawBoat(cr, 60);
}

void renderGpsTrajectoryToSvg(
    const TimedSampleRange<GeographicPosition<double>> &positions,
    NavDataset ds,
    const std::string &filename) {
  auto middle = positions[positions.size()/2];

  GeographicReference ref(middle.value);

  PlotUtils::Settings2d settings;
  settings.width = 800;
  settings.height = 600;

  auto surface = sharedPtrWrap(
      cairo_svg_surface_create(
          filename.c_str(),
          settings.width,
          settings.height));
  auto cr = sharedPtrWrap(cairo_create(surface.get()));

  auto p2 = positionsToPlane(positions, ref);

  BBox3d bbox;
  for (auto p: p2) {
    double xyz[3] = {p[0]/lengthUnit, p[1]/lengthUnit, 0.0};
    bbox.extend(xyz);
  }

  auto mat = toCairo(
      PlotUtils::computeTotalProjection(
          bbox, settings));

  {
    WithLocalCairoContext with(cr.get());
    cairo_transform(cr.get(), &mat);
    {
      auto p = p2[0];
      cairo_move_to(cr.get(), p[0]/lengthUnit, p[1]/lengthUnit);
    }
    for (int i = 1; i < p2.size(); i++) {
      auto p = p2[i];
      cairo_line_to(cr.get(), p[0]/lengthUnit, p[1]/lengthUnit);
    }
  }
  cairo_stroke(cr.get());

  TimeStamp at = middle.time;
  {
    WithLocalCairoContext with(cr.get());
    cairo_transform(cr.get(), &mat);
    renderBoat(cr.get(), ref, middle.time, ds);
  }
}

void makeSessionIllustration(
    const NavDataset &ds,
    DOM::Node page) {
  auto positions = ds.samples<GPS_POS>();
  if (positions.empty()) {
    auto p = DOM::makeSubNode(page, "p");
    DOM::addTextNode(p, "No GPS data");
  } else {
    auto p = DOM::makeGeneratedImageNode(page, ".svg");
    renderGpsTrajectoryToSvg(positions, ds, p.toString());
  }
}

void makeAllIllustrations(
    const Setup &setup,
    const Array<NavDataset> &sessions) {
  auto page = DOM::makeBasicHtmlPage("Illustrations",
      setup.prefix, "illustrations");

  auto ul = DOM::makeSubNode(page, "ul");

  for (int i = 0; i < sessions.size(); i++) {
    auto li = DOM::makeSubNode(ul, "li");
    auto subPage = DOM::linkToSubPage(li, stringFormat("Session %d", i));
    makeSessionIllustration(sessions[i], subPage);
  }
}

bool makeIllustrations(const Setup &setup) {
  auto start = TimeStamp::now();
  BoatLogProcessor proc;

  NavDataset raw = loadNavs(setup.path)
      .fitBounds();

  raw = raw.slice(earliest(setup.from(), raw.lowerBound()),
                    latest(setup.to(), raw.upperBound()));

  auto resampled = downSampleGpsTo1Hz(raw);
  resampled = filterNavs(resampled, proc._gpsFilterSettings);

  std::shared_ptr<HTree> fulltree = proc._grammar.parse(resampled);

  if (!fulltree) {
    LOG(WARNING) << "grammar parsing failed. No data?";
    return false;
  }

  Calibrator calibrator(proc._grammar.grammar);
  if (proc._verboseCalibrator) { calibrator.setVerbose(); }
  std::string boatDatPath = setup.prefix + "/boat.dat";
  std::ofstream boatDatFile(boatDatPath);

  // Calibrate. TODO: use filtered data instead of resampled.
  if (calibrator.calibrate(resampled, fulltree, proc._boatid)) {
      calibrator.saveCalibration(&boatDatFile);
  } else {
    LOG(WARNING) << "Calibration failed. Using default calib values.";
    calibrator.clear();
  }

  // First simulation pass: adds true wind
  NavDataset simulated = calibrator.simulate(resampled);

    /*
  Why this is needed:
  Whenever the NavDataset::samples<...> method is called, a merge is performed
  for that datacode using all the data of the underlying dispatcher
  (not limited in time). Several NavDatasets will be produced by in the following
  code by slicing up the full NavDataset. Before this slicing takes place,
  we want to merge the data, so that it doesn't have to be merged for every
  slice that produce. This saves us a lot of memory. If we decide to refactor
  this code some time, we should think carefully how we want to do the merging.
     */
  simulated.mergeAll();

  outputTargetSpeedTable(proc._debug,
                         fulltree,
                         proc._grammar.grammar.nodeInfo(),
                         simulated,
                         proc._vmgSampleSelection,
                         &boatDatFile);
  // write calibration and target speed to disk
  boatDatFile.close();

  // Second simulation path to apply target speed.
  // Todo: simply lookup the target speed instead of recomputing true wind.
  simulated = SimulateBox(boatDatPath, simulated);

  if (proc._debug) {
    visualizeBoatDat(setup.prefix);
  }

  Array<NavDataset> sessions =
    extractAll("Sailing", simulated, proc._grammar.grammar, fulltree);

  makeAllIllustrations(setup, sessions);

  LOG(INFO) << "Processing time for " << proc._boatid << ": "
    << (TimeStamp::now() - start).seconds() << " seconds.";
  return true;
}

int main(int argc, const char **argv) {

  Setup setup;
  ArgMap amap;
  amap.registerOption("--path", "Path to dataset")
      .store(&(setup.path))
      .required();
  amap.registerOption("--from", "From date YYYY-MM-DD").store(&(setup.toStr));
  amap.registerOption("--to", "To date YYYY-MM-DD").store(&(setup.fromStr));

  auto status = amap.parse(argc, argv);
  switch (status) {
  case ArgMap::Done:
    return 0;
  case ArgMap::Continue:
    return makeIllustrations(setup)? 0 : -2;
  case ArgMap::Error:
    return -1;
  };
  return 0;
}
