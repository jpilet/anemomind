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
using namespace sail::Cairo;

std::default_random_engine rng;

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

  Array<TimeStamp> selected;
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


void renderBoat(
    BoatRenderSettings rs,
    cairo_t *cr,
    const GeographicReference &ref,
    TimeStamp time,
  const NavDataset &ds) {

  WithLocalContext withContext(cr);

  auto pos = ds.samples<GPS_POS>().nearest(time);
  if (!pos.defined()) {
    std::cout << "Missing pos";
    return;
  }

  auto hdg = ds.samples<MAG_HEADING>().nearest(time);
  if (!hdg.defined()) {
    std::cout << "Missing heading";
    return;
  }

  auto localPos = ref.map(pos.get().value);

  cairo_translate(cr,
      localPos[0]/lengthUnit,
      localPos[1]/lengthUnit);
  WithLocalDeviceScale withDevScale(cr,
      WithLocalDeviceScale::Determinant);

  {
    WithLocalContext context(cr);
    rotateGeographically(cr, hdg.get().value);
    setSourceColor(cr, rs.boat);
    drawBoat(cr, 60);
  }{
    auto twdir = ds.samples<TWDIR>().nearest(time);
    if (!twdir.defined()) {
      std::cout << "Missing twdir" << std::endl;
      return;
    }
    auto tws = ds.samples<TWS>().nearest(time);
    if (!tws.defined()) {
      std::cout << "Missing tws" << std::endl;
      return;
    }
    auto motion = HorizontalMotion<double>::polar(
        tws.get().value,
        twdir.get().value + Angle<double>::degrees(180.0));

    auto coef = (4.0_m/1.0_kn)/lengthUnit;

    Eigen::Vector2d flow(double(motion[0]*coef), double(motion[1]*coef));
    drawLocalFlow(cr, flow, 60.0, 9, 0.2*flow.norm(), &rng);
  }
}

void renderGpsTrajectoryToSvg(
    const TimedSampleRange<GeographicPosition<double>> &positions,
    NavDataset ds,
    const std::string &filename,
    const std::vector<TimeStamp> &times) {
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
    WithLocalContext with(cr.get());
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


  BoatRenderSettings rs;
  for (auto t: times) {
    WithLocalContext with(cr.get());
    cairo_transform(cr.get(), &mat);
    renderBoat(rs, cr.get(), ref, t, ds);
  }
}

void makeSessionIllustration(
    const NavDataset &ds,
    DOM::Node page,
    const std::vector<TimeStamp> &times) {
  auto positions = ds.samples<GPS_POS>();
  if (positions.empty()) {
    auto p = DOM::makeSubNode(page, "p");
    DOM::addTextNode(p, "No GPS data");
  } else {
    auto p = DOM::makeGeneratedImageNode(page, ".svg");
    renderGpsTrajectoryToSvg(positions, ds, p.toString(),
        times);
  }
}

int findSessionWithTimeStamp(
    const Array<NavDataset> &sessions,
    TimeStamp t) {
  for (int i = 0; i < sessions.size(); i++) {
    auto s = sessions[i];
    if (s.lowerBound() <= t && t < s.upperBound()) {
      return i;
    }
  }
  return -1;
}

void makeAllIllustrations(
    const Setup &setup,
    const Array<NavDataset> &sessions) {
  auto page = DOM::makeBasicHtmlPage("Illustrations",
      setup.prefix, "illustrations");

  Array<std::vector<TimeStamp>> selPerSession(sessions.size());

  if (!setup.selected.empty()) {
    auto p = DOM::makeSubNode(page, "p");
    DOM::addTextNode(p, "You want to look closer at ");
    auto ul = DOM::makeSubNode(page, "ul");
    for (auto t: setup.selected) {
      int index = findSessionWithTimeStamp(sessions, t);
      DOM::addSubTextNode(ul, "li", t.toString()
          + stringFormat(" belonging to session %d",
              1+index));

      if (index != -1) {
        selPerSession[index].push_back(t);
      }
    }
  }

  auto table = DOM::makeSubNode(page, "table");
  {
    auto row = DOM::makeSubNode(table, "tr");
    DOM::addSubTextNode(row, "th", "Index");
    DOM::addSubTextNode(row, "th", "From");
    DOM::addSubTextNode(row, "th", "To");
    DOM::addSubTextNode(row, "th", "Selected count");
  }

  for (int i = 0; i < sessions.size(); i++) {
    const auto &sel = selPerSession[i];
    auto session = sessions[i];
    auto row = DOM::makeSubNode(table, "tr");
    auto title = stringFormat("Session %d", i);
    {
      auto td = makeSubNode(row, "td");
      auto subPage = DOM::linkToSubPage(td, title);
      makeSessionIllustration(session, subPage, sel);
    }
    DOM::addSubTextNode(row, "td",
        session.lowerBound().toString());
    DOM::addSubTextNode(row, "td",
        session.upperBound().toString());
    DOM::addSubTextNode(row, "td",
        stringFormat("%d", sel.size()));
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

namespace {
  Array<TimeStamp> getSelectedTimeStamps(
      ArgMap *amap) {
    auto opts = amap->optionArgs("--select");
    ArrayBuilder<TimeStamp> dst;
    for (auto opt: opts) {
      auto s = opt->value();
      TimeStamp t = TimeStamp::parse(s);
      if (!t.defined()) {
        LOG(FATAL) << "Failed to parse time " << s;
      } else {
        dst.add(t);
      }
    }
    return dst.get();
  }

}


int main(int argc, const char **argv) {

  Setup setup;
  ArgMap amap;
  amap.registerOption("--path", "Path to dataset")
      .store(&(setup.path))
      .required();
  amap.registerOption("--from", "From date YYYY-MM-DD").store(&(setup.toStr));
  amap.registerOption("--to", "To date YYYY-MM-DD").store(&(setup.fromStr));
  amap.registerOption("--select",
      "Select session, date on from YYYY-MM-DD");



  auto status = amap.parse(argc, argv);
  switch (status) {
  case ArgMap::Done:
    return 0;
  case ArgMap::Continue:
    setup.selected = getSelectedTimeStamps(&amap);
    return makeIllustrations(setup)? 0 : -2;
  case ArgMap::Error:
    return -1;
  };
  return 0;
}
