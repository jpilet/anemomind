/*
 * illustrate.cpp
 *
 *  Created on: 13 Oct 2016
 *      Author: jonas
 */

#include <server/common/CmdArg.h>
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
#include <server/html/HtmlDispatcher.h>
#include <server/common/LineKM.h>

using namespace sail;
using namespace sail::Cairo;

std::default_random_engine rng;

namespace {
  auto lengthUnit = 1.0_m;
  auto flowCoef = (4.0_m/1.0_kn)/lengthUnit;
}

enum class RenderMode {
  PerBoat,
  PerTrajectory
};




struct LocalFlowSettings {
  int arrowCount = 9;
  double localStdDev = 60.0;
  decltype(flowCoef) coef = flowCoef;
  double pointSize = 0.2;
  Eigen::Vector2d toRawVector(
      const HorizontalMotion<double> &motion) const;
};

Eigen::Vector2d LocalFlowSettings::toRawVector(
    const HorizontalMotion<double> &motion) const {
  return Eigen::Vector2d(
      double(motion[0]*coef),
      double(motion[1]*coef));
}

LocalFlowSettings makeTrajFlow() {
  LocalFlowSettings s;
  s.arrowCount = 1;
  s.localStdDev = 30;
  return s;
}

struct Setup {
  LocalFlowSettings localFlowSettings, trajFlow = makeTrajFlow();
  RenderMode mode = RenderMode::PerTrajectory;
  std::string path;
  TimeStamp from, to;
  std::string outputName = "illustrations";
  std::string prefix = PathBuilder::makeDirectory(
      Env::BINARY_DIR).get().toString();
  std::string name = "illustrate";

  std::vector<TimeStamp> selected;

  Duration<double> maneuverMarg = 2.0_minutes;
  Duration<double> margin;
  Duration<double> windSamplingPeriod = 10.0_s;

  PlotUtils::RGB boatColor(int index) const;
};

PlotUtils::RGB Setup::boatColor(int index) const {
  LineKM hueMap(0.0, selected.size(), 0.0, 360.0);
  return PlotUtils::hsv2rgb(
      PlotUtils::HSV::fromHue(hueMap(index)*1.0_deg));
}

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


typedef std::function<Optional<HorizontalMotion<double>>(TimeStamp)> WindFunction;

struct WindToRender {
  PlotUtils::RGB windColor;
  WindFunction wind;
};

struct BoatPose {
  TimeStamp t;
  GeographicPosition<double> pos;
  Angle<double> heading;
};

Optional<BoatPose> getBoatPose(TimeStamp t,
    const NavDataset &ds) {
  auto pos = ds.samples<GPS_POS>().nearest(t);
  if (!pos.defined()) {
    std::cout << "Missing pos";
    return Optional<BoatPose>();
  }

  auto hdg = ds.samples<MAG_HEADING>().nearest(t);
  if (!hdg.defined()) {
    std::cout << "Missing heading";
    return Optional<BoatPose>();
  }
  return BoatPose{
    t, pos.get().value, hdg.get().value
  };
}

Optional<HorizontalMotion<double>> getTrueWindByTwdir(
    TimeStamp time,
    const NavDataset &ds) {
  auto twdir = ds.samples<TWDIR>().nearest(time);
  if (!twdir.defined()) {
    return Optional<HorizontalMotion<double>>();
  }
  auto tws = ds.samples<TWS>().nearest(time);
  if (!tws.defined()) {
    return Optional<HorizontalMotion<double>>();
  }
  return HorizontalMotion<double>::polar(
      tws.get().value,
      twdir.get().value + Angle<double>::degrees(180.0));
}

Optional<HorizontalMotion<double>> getTrueWindFromRaw(
    TimeStamp time, const NavDataset &ds
    /*const Dispatcher *raw*/) {
  /*auto awa = raw->get<AWA>()->dispatcher()->values().nearest(time);
  auto aws = raw->get<AWS>()->dispatcher()->values().nearest(time);
  auto magHdg = raw->get<MAG_HEADING>()->dispatcher()->values().nearest(time);
  auto gpsSpeed = raw->get<GPS_SPEED>()->dispatcher()->values().nearest(time);
  auto gpsBearing = raw->get<GPS_BEARING>()->dispatcher()->values().nearest(time);

  if (awa.defined() && aws.defined() && magHdg.defined()
      && gpsSpeed.defined() && gpsBearing.defined()) {


    auto gpsMotion = HorizontalMotion<double>::polar(
        gpsSpeed.get(), gpsBearing.get());
    auto aw = HorizontalMotion<double>::polar(
        aws.get(), 180.0_deg + awa.get() + magHdg.get());

    return HorizontalMotion<double>(gpsMotion + aw);
  }
  return Optional<HorizontalMotion<double>>();*/
  auto nav = NavCompat::getNavByTime(ds, time);
  return HorizontalMotion<double>::polar(
      nav.externalTws(), 180.0_deg + nav.externalTwa() + nav.gpsBearing());

}

WindToRender makeCalibratedTrueWind(const NavDataset &d) {
  using namespace PlotUtils;
  return WindToRender{
    hsv2rgb(HSV::fromHue(120.0_deg)),
        [=](TimeStamp t) {return getTrueWindByTwdir(t, d);}
  };
}

WindToRender makeRawTrueWind(const NavDataset &ds) {
  using namespace PlotUtils;
  return WindToRender{
    hsv2rgb(HSV::fromHue(30.0_deg)), // Orange
    [=](TimeStamp t) {return getTrueWindFromRaw(t, ds);}
  };
}

struct BoatToRender {
  GeographicPosition<double> position;
  Angle<double> heading;

  PlotUtils::RGB boatColor = PlotUtils::hsv2rgb(
      PlotUtils::HSV::fromHue(215.0_deg));

  std::vector<WindToRender> winds;
};

void renderBoat(
    const Setup &setup,
    BoatToRender rs,
    cairo_t *cr,
    const GeographicReference &ref,
    TimeStamp time) {

  WithLocalContext withContext(cr);
  setSourceColor(cr, rs.boatColor);

  auto localPos = ref.map(rs.position);

  cairo_translate(cr,
      localPos[0]/lengthUnit,
      localPos[1]/lengthUnit);
  WithLocalDeviceScale withDevScale(cr,
      WithLocalDeviceScale::Determinant);

  {
    WithLocalContext context(cr);
    rotateGeographically(cr, rs.heading);
    drawBoat(cr, 60);
  }


  for (auto wind: rs.winds) {
    WithLocalContext withContext(cr);
    setSourceColor(cr, wind.windColor);
    auto motion0 = wind.wind(time);
    if (motion0.defined()) {
      auto motion = motion0.get();

      auto flow = setup.localFlowSettings.toRawVector(motion);
      drawLocalFlow(cr, flow,
          setup.localFlowSettings.localStdDev,
          setup.localFlowSettings.arrowCount,
          setup.localFlowSettings.pointSize*flow.norm(), &rng);
    }
  }
}

TimeStamp getRandomTime(TimeStamp from, TimeStamp to) {
  std::uniform_real_distribution<double> distrib(0.0, 1.0);
  return from + distrib(rng)*(to - from);
}

void drawLocalWinds(
    const Setup &setup,
    const GeographicReference &geoRef,
    cairo_t *cr,
    int sampleCount, TimeStamp firstTime,
    TimeStamp lastTime,
    const TimedSampleRange<GeographicPosition<double>> &positions,
    const WindToRender &windToRender) {
  WithLocalContext with(cr);
  Cairo::setSourceColor(cr, windToRender.windColor);
  for (int i = 0; i < sampleCount; i++) {
    auto time = getRandomTime(firstTime, lastTime);
    auto gpos = positions.nearest(time);
    auto wind = windToRender.wind(time);
    if (gpos.defined() && wind.defined()) {
      auto pos = geoRef.map(gpos.get().value);
      WithLocalContext with(cr);
      cairo_translate(cr, pos[0]/lengthUnit, pos[1]/lengthUnit);
      auto flow = setup.trajFlow.toRawVector(wind.get());
      drawLocalFlow(cr, flow,
          setup.trajFlow.localStdDev, setup.trajFlow.arrowCount,
          setup.trajFlow.pointSize*flow.norm(), &rng);
    }
  }
}


void renderGpsTrajectoryToSvg(
    const Setup &setup,
    const TimedSampleRange<GeographicPosition<double>> &positions,
    NavDataset ds,
    const std::string &filename,
    const std::vector<TimeStamp> &selectedTimes,
    const std::vector<ManeuverData> &maneuverTimes,
    const WindToRender &rawTrueWind,
    const WindToRender &calibratedTrueWind) {

  std::cout << "renderGpsTrajectoryToSvg" << std::endl;

  if (positions.empty()) {
    return;
  }

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

  Eigen::Matrix<double, 2, 4> proj =
      PlotUtils::computeTotalProjection(bbox, settings);
  auto mat = sail::Cairo::toCairo(proj);

  std::cout << "Make the traj..." << std::endl;

  WithLocalContext with(cr.get());
  cairo_transform(cr.get(), &mat);
  {
    WithLocalContext with(cr.get());
    {
      auto p = p2[0];
      cairo_move_to(cr.get(), p[0]/lengthUnit, p[1]/lengthUnit);
    }
    for (int i = 1; i < p2.size(); i++) {
      auto p = p2[i];
      cairo_line_to(cr.get(), p[0]/lengthUnit, p[1]/lengthUnit);
    }
    WithLocalDeviceScale wds(cr.get(), WithLocalDeviceScale::Determinant);
    cairo_stroke(cr.get());
  }

  for (auto t: maneuverTimes) {
    WithLocalContext wlc(cr.get());
    cairo_set_line_width(cr.get(), 4);
    Cairo::setSourceColor(cr.get(), PlotUtils::HSV::fromHue(240.0_deg));
    auto px = positions.nearest(t.time);
    if (px.defined()) {
      auto p = ref.map(px.get().value);
      cairo_translate(cr.get(),
          double(p[0]/lengthUnit), double(p[1]/lengthUnit));
      Cairo::drawCross(cr.get(), 20);
    }
  }


  if (setup.mode == RenderMode::PerTrajectory) {
    WithLocalContext with(cr.get());
    TimeStamp firstTime = positions.first().time;
    TimeStamp lastTime = positions.last().time;
    int sampleCount = int(ceil((lastTime - firstTime)/setup.windSamplingPeriod));
    drawLocalWinds(setup, ref, cr.get(), sampleCount, firstTime,
        lastTime, positions, rawTrueWind);
    drawLocalWinds(setup, ref, cr.get(), sampleCount, firstTime,
        lastTime, positions, calibratedTrueWind);
  }




  for (int i = 0; i < selectedTimes.size(); i++) {
    auto pose0 = getBoatPose(selectedTimes[i], ds);
    BoatToRender rs;
    if (pose0.defined()) {
      auto pose = pose0.get();
      rs.position = pose.pos;
      rs.heading = pose.heading;
      if (setup.mode == RenderMode::PerBoat) {
        rs.boatColor = setup.boatColor(i);
        rs.winds.push_back(rawTrueWind);
        rs.winds.push_back(calibratedTrueWind);
      } else {
        rs.boatColor = PlotUtils::RGB();
      }
      auto t = selectedTimes[i];
      WithLocalContext with(cr.get());
      renderBoat(setup, rs, cr.get(), ref, t);
    }
  }
}

NavDataset cropDataset(const NavDataset &ds,
    const std::vector<TimeStamp> &times,
    Duration<double> marg) {
  TimeStamp lower, upper;
  for (auto t: times) {
    lower = earliest(lower, t);
    upper = latest(upper, t);
  }
  return ds.slice(lower - marg, upper + marg);
}

void makeSessionIllustration(
    const Setup &setup,
    const NavDataset &ds0,
    DOM::Node page,
    const std::vector<TimeStamp> &selectedTimes,
    const std::vector<ManeuverData> &maneuverTimes,
    const WindToRender &rawTrueWind,
    const WindToRender &calibratedTrueWind) {
  std::cout << "What about selected times..." <<
      std::string(selectedTimes.empty()? "Emtpy" : "have data") << std::endl;

  auto ds = selectedTimes.empty()? ds0 : cropDataset(ds0, selectedTimes, setup.margin);

  std::cout << "Make the overview..." << std::endl;
  renderDispatcherTableOverview(
      ds.dispatcher().get(), &page);

  std::cout << "Get the positions" << std::endl;

  auto positions = ds.samples<GPS_POS>();
  if (positions.empty()) {
    auto p = DOM::makeSubNode(&page, "p");
    DOM::addTextNode(&p, "No GPS data");
  } else {
    auto p = DOM::makeGeneratedImageNode(page, ".svg");
    std::cout << "Render gps trajectory" << std::endl;
    renderGpsTrajectoryToSvg(
        setup,
        positions, ds, p.toString(),
        selectedTimes,
        maneuverTimes, rawTrueWind, calibratedTrueWind);
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

TimeStamp timeOf(TimeStamp x) {return x;}
TimeStamp timeOf(const ManeuverData &d) {return d.time;}

template <typename T>
Array<std::vector<T>> assignTimeStampToSession(
    const Array<NavDataset> &sessions,
    const Array<T> &selected,
    DOM::Node *page) {
  Array<std::vector<T>> selPerSession(sessions.size());

  if (!selected.empty()) {
    auto ul = DOM::makeSubNode(page, "ul");
    for (auto t: selected) {
      int index = findSessionWithTimeStamp(sessions, timeOf(t));
      DOM::addSubTextNode(&ul, "li", timeOf(t).toString()
          + stringFormat(" belonging to session %d",
              1+index));
      if (index != -1) {
        selPerSession[index].push_back(t);
      }
    }
  }
  return selPerSession;
}


void makeAllIllustrations(
    DOM::Node page,
    const Setup &setup,
    const Array<NavDataset> &sessions,
    const Array<ManeuverData> &tackTimes,
    const WindToRender &rawTrueWind,
    const WindToRender &calibratedTrueWind) {

  std::cout << "Make all illustrations..." << std::endl;

  Array<std::vector<TimeStamp>> selPerSession;
  DOM::addSubTextNode(&page, "p", "You want to look closer at ");
  selPerSession = assignTimeStampToSession(
      sessions, Array<TimeStamp>::referToVector(setup.selected), &page);
  DOM::addSubTextNode(&page, "p", "Maneuvers:");
  auto mansPerSession = assignTimeStampToSession(sessions, tackTimes, &page);

  {
    DOM::addSubTextNode(&page, "h2", "Per session plots, with selection");
    auto table = DOM::makeSubNode(&page, "table");
    {
      auto row = DOM::makeSubNode(&table, "tr");
      DOM::addSubTextNode(&row, "th", "Index");
      DOM::addSubTextNode(&row, "th", "From");
      DOM::addSubTextNode(&row, "th", "To");
      DOM::addSubTextNode(&row, "th", "Selected count");
    }

    for (int i = 0; i < sessions.size(); i++) {
      std::cout << "Illustrate session " << i << std::endl;
      const auto &sel = selPerSession[i];
      const auto &man = mansPerSession[i];
      auto session = sessions[i];
      auto row = DOM::makeSubNode(&table, "tr");
      std::cout << "Make table title" << std::endl;
      auto title = stringFormat("Session %d", i);
      {
        auto td = makeSubNode(&row, "td");
        auto subPage = DOM::linkToSubPage(td, title);
        std::cout << "Make session illustration" << std::endl;
        makeSessionIllustration(
            setup, session, subPage, sel, man,
            rawTrueWind, calibratedTrueWind);
      }
      DOM::addSubTextNode(&row, "td",
          session.lowerBound().toString());
      DOM::addSubTextNode(&row, "td",
          session.upperBound().toString());
      DOM::addSubTextNode(&row, "td",
          stringFormat("%d", sel.size()));
    }
  }{
    DOM::addSubTextNode(&page, "h2", "Per maneuver-plots");
    auto table = DOM::makeSubNode(&page, "table");
    {
      auto row = DOM::makeSubNode(&table, "tr");
      DOM::addSubTextNode(&row, "th", "At time");
      DOM::addSubTextNode(&row, "th", "Session index");
      DOM::addSubTextNode(&row, "th", "Angle error");
    }{
      for (int i = 0; i < sessions.size(); i++) {
        auto session = sessions[i];
        auto man = mansPerSession[i];
        for (auto maneuverTime: man) {
          auto row = DOM::makeSubNode(&table, "tr");
          auto td = makeSubNode(&row, "td");
          auto title = maneuverTime.time.toString();
          auto subPage = DOM::linkToSubPage(td, title);
          makeSessionIllustration(
              setup, session,
              subPage, std::vector<TimeStamp>{
                  maneuverTime.time - 1.0_minutes,
                  maneuverTime.time + 1.0_minutes},
              std::vector<ManeuverData>{},
              rawTrueWind, calibratedTrueWind);
          DOM::addSubTextNode(&row, "td", stringFormat("%d", i));
          DOM::addSubTextNode(&row, "td", stringFormat("%.3g", maneuverTime.angleError));
        }
      }
    }
  }
}

bool makeIllustrations(const Setup &setup) {
  auto start = TimeStamp::now();
  BoatLogProcessor proc;

  NavDataset raw = loadNavs(setup.path)
      .fitBounds();

  auto rawTrueWind = makeRawTrueWind(raw);

  raw = raw.slice(earliest(setup.from, raw.lowerBound()),
                    latest(setup.to, raw.upperBound()));

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
  auto tackData = calibrator.maneuverData();

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

  auto calibratedTrueWind = makeCalibratedTrueWind(simulated);

  if (proc._debug) {
    visualizeBoatDat(setup.prefix);
  }

  Array<NavDataset> sessions =
    extractAll("Sailing", simulated, proc._grammar.grammar, fulltree);

  std::cout << "MAKE THE ILLUSTRATIONS!" << std::endl;
  auto page = DOM::makeBasicHtmlPage("Illustrations",
      setup.prefix, setup.outputName);
  sail::renderDispatcherTableOverview(
      raw.dispatcher().get(), &page);
  makeAllIllustrations(page, setup, sessions,
      tackData, rawTrueWind, calibratedTrueWind);

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

  std::map<std::string, Duration<double>>
    timeUnits{
      {"s", 1.0_s},
      {"min", 1.0_minutes},
    };


  Setup setup;
  using namespace CmdArg;

  Parser parser("Illustrations");

  parser.bind({"--margin"}, {

      inputForm([&](double t, std::string u) {
        if (timeUnits.count(u) == 0) {
          return false;
        }
        setup.margin = t*timeUnits[u];
        return true;
      }, Arg<double>("value"),
         Arg<std::string>("unit").describe("'s' or 'min'"))
         .describe("Value and unit")

  }).describe("Time margin around selected points");

  parser.bind({"--path"}, {
      inputForm([&](const std::string &p) {
        setup.path = p;
        return true;
      }, Arg<std::string>("path"))
  }).describe("Path to dataset");

  parser.bind({"--from"}, {
      inputForm([&](const std::string &p) {
        auto t = TimeStamp::parse(p);
        if (!t.defined()) {
          return Result::failure("Failed to parse date " + p);
        }
        setup.from = t;
        return Result::success();
      }, Arg<std::string>("date").describe("YYYY-MM-DD"))
  }).describe("From where we should cut");


  parser.bind({"--to"}, {
      inputForm([&](const std::string &p) {
        auto t = TimeStamp::parse(p);
        if (!t.defined()) {
          return Result::failure("Failed to parse date " + p);
        }
        setup.to = t;
        return Result::success();
      }, Arg<std::string>("date").describe("YYYY-MM-DD"))
  }).describe("To where we should cut");

  parser.bind({"--select"}, {
      inputForm([&](const std::string &s) {
        auto t = TimeStamp::parse(s);
        if (!t.defined()) {
          return Result::failure("Failed to parse date " + s);
        }
        setup.selected.push_back(t);
        return Result::success();
      }, Arg<std::string>("date").describe("YYYY-MM-DD"))
  }).describe("Add a selected date");

  auto status = parser.parse(argc, argv);
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
