/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_MINCOVCALIB_H_
#define SERVER_NAUTICAL_MINCOVCALIB_H_

#include <server/math/SignalCovariance.h>
#include <device/Arduino/libraries/Corrector/Corrector.h>
#include <server/nautical/Nav.h>


namespace sail {
namespace MinCovCalib {

bool hasAllData(const Nav &x);

struct Settings {
  Settings() : balanced(false) {}
  SignalCovariance::Settings covarianceSettings;

  // The various signal pairs are balanced against each other,
  // based on averaged standard deviations.
  bool balanced;
};

Corrector<double> optimize(Array<Nav> navs, Settings s);

}
}

#endif /* SERVER_NAUTICAL_MINCOVCALIB_H_ */
