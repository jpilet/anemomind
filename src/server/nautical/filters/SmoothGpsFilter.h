/*
 * SmoothGPSFilter.h
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_FILTERS_SMOOTHGPSFILTER_H_
#define SERVER_NAUTICAL_FILTERS_SMOOTHGPSFILTER_H_

#include <server/nautical/NavDataset.h>

namespace sail {

std::shared_ptr<Dispatcher> filterGpsData(const NavDataset &ds);

}

#endif /* SERVER_NAUTICAL_FILTERS_SMOOTHGPSFILTER_H_ */
