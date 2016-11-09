/*
 *  Created on: 2014-09-02
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */
#include <device/Arduino/libraries/TrueWindEstimator/TrueWindEstimator.h>
#include <server/common/Env.h>
#include <server/common/logging.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/plot/extra.h>
#include <sstream>
#include <string>
#include <algorithm>
#include <server/nautical/logimport/TestdataNavs.h>
#include <server/common/Functional.h>

using std::string;
using namespace sail;
using namespace sail::NavCompat;


namespace {
  HorizontalMotion<double> estimateRawTrueWind(const Nav &nav, Angle<double> boatDir) {
    HorizontalMotion<double> boatMotion = nav.gpsMotion();
    HorizontalMotion<double> apparentWind = HorizontalMotion<double>::polar(nav.aws(),
        nav.awa()
          + Angle<double>::radians(M_PI) // We measure the angle to where the wind is coming from
          + boatDir);                    // The AWA angle is relative to the boat heading

    HorizontalMotion<double> trueWind = apparentWind + boatMotion;
    return trueWind;
  }

  HorizontalMotion<double> estimateTrueWindUsingEstimator(const Nav &nav) {
    double parameters[TrueWindEstimator::NUM_PARAMS];
    TrueWindEstimator::initializeParameters(parameters);
    return TrueWindEstimator::computeTrueWind
      <double>(parameters, nav);
  }

  Array<Angle<double> > makeContinuous(Array<Angle<double> > X) {
    int count = X.size();
    Array<Angle<double> > Y(count);
    Y[0] = X[0];
    for (int i = 1; i < count; i++) {
      Y[i] = Y[i-1] + (X[i] - Y[i-1]).normalizedAt0();
    }
    return Y;
  }

  Arrayd toDouble(Array<Angle<double> > X) {
    return toArray(sail::map(X, [](Angle<double> x) {return x.degrees();}));
  }


  Angle<double> getMedianAbsValue(Array<Angle<double> > difs0) {
    Array<Angle<double> > difs = toArray(sail::map(difs0,
        [&](Angle<double> x) {return fabs(x);}));
    std::sort(difs.begin(), difs.end());
    return difs[difs.size()/2];
  }
}

int main(int argc, const char **argv) {
  ArgMap amap;
  amap.setHelpInfo("A program to compare the true wind read from file with the \ncalculated true wind, just for debugging. No calibration is performed.");
  amap.registerOption("--etwa", "Plot TWA from file");
  amap.registerOption("--twa", "Plot computed TWA");
  amap.registerOption("--awa", "Plot AWA");
  amap.registerOption("--difs", "Plot absolute differences between computed TWA and TWA from file");
  amap.registerOption("--twdir", "Plot computed TWDIR");
  amap.registerOption("--etwdir", "Plot TWDIR from file");
  amap.registerOption("--example-ds", "Select an example dataset (1, 2 or 3)").setArgCount(1).unique();
  registerGetTestdataNavs(amap);

  if (amap.parse(argc, argv) != ArgMap::Error) {
    if (amap.optionProvided("--help")) { // no need to parse anything if help was provided.
      return 0;
    }

    const int exdsCount = 3;
    std::string exds[exdsCount] = {
        string("/datasets/Irene/2007/regate_1_dec_07/IreneLog.txt"), // BAD
        string("/datasets/Irene/2008/regate_28_mai_08/IreneLog.txt"),  // GOOD
        string("/datasets/psaros33_Banque_Sturdza/2014/20140627/NMEA0006.TXT") // GOOD
       };

    NavDataset navs;
    if (amap.optionProvided("--example-ds")) {
      int index = -1;
      if (amap.optionArgs("--example-ds")[0]->tryParseInt(&index)) {
        if (1 <= index && index <= exdsCount) {
          auto p = string(Env::SOURCE_DIR) +
                       exds[index-1];
          navs = LogLoader::loadNavDataset(p);
        } else {
          std::cout << "Argument --example-ds out of range" << std::endl;
          return -1;
        }
      } else {
        std::cout << "Failed to parse int" << std::endl;
        return -1;
      }
    } else {
      navs = getTestdataNavs(amap);
    }
    if (isEmpty(navs)) {
      std::cout << "No navs loaded" << std::endl;
      return -1;
    }


    bool useEstimator = true;

    int count = getNavSize(navs);
    Angle<double> tol = Angle<double>::degrees(5.0);
    int counter = 0;


    Array<Angle<double> > difs(count);
    Array<Angle<double> > ETWA(count);
    Array<Angle<double> >  TWA(count);
    Array<Angle<double> >  AWA(count);
    Array<Angle<double> > ETWDIR(count);
    Array<Angle<double> >  TWDIR(count);

    Arrayd X(count);
    for (int i = 0; i < count; i++) {
      Nav nav = getNav(navs, i);
      Angle<double> boatDir = nav.gpsBearing();
      X[i] = i;

      HorizontalMotion<double> trueWind;
      if (useEstimator) {
        trueWind = estimateTrueWindUsingEstimator(nav);
      } else {
        trueWind = estimateRawTrueWind(nav, boatDir);
      }

      Angle<double> twa = calcTwa(trueWind, boatDir)
          + Angle<double>::degrees(360);

      Angle<double> twdir = trueWind.angle() + Angle<double>::degrees(180);
      TWDIR[i] = twdir;

      Angle<double> etwa = nav.externalTwa();

      Angle<double> etwdir = nav.externalTwa() + boatDir;
      ETWDIR[i] = etwdir;


      Angle<double> dif = (twa - etwa).normalizedAt0();
      if (fabs(dif) < tol) {
        counter++;
      }

      difs[i] = dif;
      TWA[i] = twa;
      ETWA[i] = etwa;
      AWA[i] = nav.awa();
    }
    double median = getMedianAbsValue(difs).degrees();

    std::cout << "Median absolute difference between twa and etwa is " << median << " degrees." << std::endl;
    std::cout << "Number of observations for which difference between twa and etwa is less than 5 degrees: " << counter << std::endl;
    std::cout << "Total number of observations: " << getNavSize(navs) << std::endl;

    GnuplotExtra plot;
    plot.set_style("lines");
    if (amap.optionProvided("--etwa")) {
      plot.plot_xy(X, toDouble(makeContinuous(ETWA)), "etwa");
    }
    if (amap.optionProvided("--twa")) {
      plot.plot_xy(X, toDouble(makeContinuous(TWA)), "twa");
    }
    if (amap.optionProvided("--awa")) {
      plot.plot_xy(X, toDouble(makeContinuous(AWA)), "awa");
    }
    if (amap.optionProvided("--difs")) {
      plot.plot_xy(X, toDouble(makeContinuous(difs)), "difs");
    }
    if (amap.optionProvided("--twdir")) {
      plot.plot_xy(X, toDouble(makeContinuous(TWDIR)), "twdir");
    }
    if (amap.optionProvided("--etwdir")) {
      plot.plot_xy(X, toDouble(makeContinuous(ETWDIR)), "etwdir");
    }
    plot.show();
  }
}



