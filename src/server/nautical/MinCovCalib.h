/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_MINCOVCALIB_H_
#define SERVER_NAUTICAL_MINCOVCALIB_H_

#include <server/math/SignalCovariance.h>
#include <device/Arduino/libraries/Corrector/Corrector.h>
#include <server/nautical/Nav.h>
#include <server/nautical/FilteredNavData.h>


namespace sail {
namespace MinCovCalib {

Array<Corrector<double> > makeCorruptCorrectors();

struct Settings {
  Settings() : balanced(false), corruptors(makeCorruptCorrectors().sliceTo(8)) {}
  SignalCovariance::Settings covarianceSettings;

  // The various signal pairs are balanced against each other,
  // based on averaged standard deviations.
  bool balanced;
  Array<Corrector<double> > corruptors;
};

/*
 * Optimize so that the covariance between wind and boat orientation is small,
 * and so that covariance between current and boat orientation is small.
 */
Corrector<double> optimizeByOrientation(FilteredNavData data, Settings s);

/*
 * No matter how the wind is calibrated, a correctly calibrated current
 * should have minimum covariance with the wind. And vice versa.
 */
Corrector<double> optimizeWindVsCurrent(FilteredNavData data, Settings s);

}
}

#endif /* SERVER_NAUTICAL_MINCOVCALIB_H_ */
