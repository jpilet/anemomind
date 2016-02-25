/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/nautical/NavDataset.h>

namespace sail {

NavDataset::NavDataset(const std::shared_ptr<Dispatcher> dispatcher) :
    _dispatcher(dispatcher) {}

}
