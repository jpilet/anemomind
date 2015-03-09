/*
 *  Created on: 2015-03-09
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Various real datasets that can be used for evaluation of calibration algorithms.
 */

#ifndef SERVER_NAUTICAL_REALCALIBDATA_H_
#define SERVER_NAUTICAL_REALCALIBDATA_H_

#include <Poco/Path.h>
#include <server/common/Array.h>

namespace sail {

Array<Poco::Path> getRealDatasets();

}

#endif
