/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TESTDATANAVS_H_
#define TESTDATANAVS_H_

#include <server/nautical/Nav.h>
#include <server/common/ArgMap.h>

namespace sail {

/*
 * Returns the navs from the Irene dataset.
 */
NavCollection getTestdataNavs();

/*
 * Let's the user provide an optional path to
 * the navs, by prefixing that path on the command
 * line with --navpath, e.g.
 *
 * --navpath /home/jonas/my-navs
 *
 * If now path is provided, a default set of Navs is returned.
 */

void registerGetTestdataNavs(ArgMap &amap);
NavCollection getTestdataNavs(ArgMap &amap);

} /* namespace sail */

#endif /* TESTDATANAVS_H_ */
