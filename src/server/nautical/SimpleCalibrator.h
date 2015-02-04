/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_SIMPLECALIBRATOR_H_
#define SERVER_NAUTICAL_SIMPLECALIBRATOR_H_

#include <server/math/Integral1d.h>
#include <server/nautical/FilteredNavData.h>
#include <server/nautical/Corrector.h>

namespace sail {

class SimpleCalibrator {
 public:
  SimpleCalibrator() : _integrationWidth(Duration<double>::seconds(12)),
                       _gap(Duration<double>::seconds(12)),
                       _maneuverCount(30) {}

  Corrector<double> calibrate(FilteredNavData data) const;
 private:
  Duration<double> _integrationWidth, _gap;
  int _maneuverCount;
};

}

#endif /* SERVER_NAUTICAL_SIMPLECALIBRATOR_H_ */
