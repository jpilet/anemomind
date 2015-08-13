/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Revised approach to segment navs into sailing sessions.
 */

#ifndef SERVER_NAUTICAL_SESSIONS_H_
#define SERVER_NAUTICAL_SESSIONS_H_

#include <server/nautical/Nav.h>
#include <server/nautical/TestdataNavs.h>
#include <iosfwd>

namespace sail {
namespace Sessions {

struct Session {
 Array<Nav> navs;
 Duration<double> averageSamplingPeriod, offset;
};

Array<Session> segment(Array<Nav> navs, Duration<double> maxRms);

std::ostream &operator<<(std::ostream &dst, const Session &x);

}
}

#endif
