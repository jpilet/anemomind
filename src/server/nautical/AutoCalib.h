/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef AUTOCALIB_H_
#define AUTOCALIB_H_

#include <cmath>
#include <server/nautical/FilteredNavData.h>
#include <device/Arduino/libraries/CalibratedNav/CalibratedNav.h>
#include <server/math/nonlinear/LevmarSettings.h>
#include <server/nautical/Corrector.h>

namespace sail {

class AutoCalib {
 public:
  // Holds settings related to how we calibrate
  class Settings {
   public:
    class QParam {
     public:
      enum Mode {FIXED, TUNED, TUNE_ON_ERROR};
      Mode mode;
      QParam() : mode(TUNED), fixedQuality(NAN), minCount(30),
          frac(0.01) {}


      double fixedQuality;
      int minCount;
      double frac;
    };
    enum GMode {GPS, MAG_HDG};

    Settings() : smooth(true), gMode(GPS),
        tapeIndex(0), jacobianCheck(false) {}

    // Quality parameters.
    QParam wind, current;

    // Whether a smooth or non-smooth robust estimator should be used.
    bool smooth;

    // Whether we should use the derivative of the GPS speed or the derivative
    // of the magnetic heading to measure how much the trajectory is changing.
    GMode gMode;

    short int tapeIndex;

    bool jacobianCheck;
  };

  static LevmarSettings makeDefaultLevmarSettings();

  AutoCalib(Settings s = Settings(), LevmarSettings optSettings = makeDefaultLevmarSettings()) :
    _settings(s), _optSettings(optSettings) {}

  class Results {
   public:
    Results(Corrector<double> corr, FilteredNavData srcData) :
      _calibratedCorrector(corr), _srcData(srcData) {}

    void disp(std::ostream *dst = nullptr);
   private:
   Corrector<double> _calibratedCorrector;
    FilteredNavData _srcData;
  };

  Results calibrate(FilteredNavData data, Arrayd times = Arrayd()) const;
 private:
  Settings _settings;
  LevmarSettings _optSettings;
};

}

#endif /* AUTOCALIB_H_ */
