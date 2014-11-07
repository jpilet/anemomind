/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef AUTOCALIB_H_
#define AUTOCALIB_H_

#include <cmath>

namespace sail {

class AutoCalib {
 public:
  AutoCalib();
  class Settings {
   public:
    class QParam {
     public:
      enum Mode {FIXED, TUNED, TUNED_ON_ERROR};
      Mode mode;
      QParam() : mode(TUNED), fixedQuality(NAN), minCount(30), frac(0.01) {}


      double fixedQuality;
      int minCount;
      double frac;
    };
    QParam wind, current;
  };

  AutoCalib(Settings s = Settings(), LevmarSettings optSettings = LevmarSettings()) :
    _settings(s), _optSettings(optSettings) {}
 private:
  Settings _settings;
  LevmarSettings _optSettings;
};

}

#endif /* AUTOCALIB_H_ */
