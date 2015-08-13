/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Revised approach to segment navs into sailing sessions.
 */

#ifndef SERVER_NAUTICAL_SESSIONS_H_
#define SERVER_NAUTICAL_SESSIONS_H_

#include <server/nautical/Nav.h>

namespace sail {
namespace Sessions {

struct Session {
 Array<Nav> navs;
 Duration<double> averageSamplingPeriod, offset;
};

Array<Session> segment(Array<Nav> navs, Duration<double> maxRms);

}
}

#endif
