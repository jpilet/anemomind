/*
 *  Created on: 28 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef NAVNMEASCAN_H_
#define NAVNMEASCAN_H_

#include <Poco/Path.h>
#include <server/nautical/Nav.h>

namespace sail {

Array<Nav> scanNmeaFolder(Poco::Path p);

} /* namespace sail */

#endif /* NAVNMEASCAN_H_ */
