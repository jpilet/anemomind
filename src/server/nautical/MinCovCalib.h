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

struct Settings {
  SignalCovariance::Settings covarianceSettings;
};

Corrector<double> optimize(Array<Nav> navs, Settings s);

}
}

#endif /* SERVER_NAUTICAL_MINCOVCALIB_H_ */
