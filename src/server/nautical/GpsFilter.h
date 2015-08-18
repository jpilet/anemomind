/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_GPSFILTER_H_
#define SERVER_NAUTICAL_GPSFILTER_H_

#include <server/nautical/Nav.h>

namespace sail {
namespace GpsFilter {

struct Settings {
  Settings();

  Duration<double> samplingPeriod;
};

Array<Nav> filter(Array<Nav> navs, Settings settings);

}
}

#endif /* SERVER_NAUTICAL_GPSFILTER_H_ */
