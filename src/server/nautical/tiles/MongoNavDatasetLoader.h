/*
 * MongoNavDatasetLoader.h
 *
 *  Created on: 6 Jul 2018
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_TILES_MONGONAVDATASETLOADER_H_
#define SERVER_NAUTICAL_TILES_MONGONAVDATASETLOADER_H_

#include <server/nautical/NavDataset.h>
#include <server/nautical/tiles/MongoUtils.h>

namespace sail {

NavDataset loadEvents(
    const NavDataset& dst,
    const MongoDBConnection& connection,
    const std::string& boatId);

} /* namespace sail */

#endif /* SERVER_NAUTICAL_TILES_MONGONAVDATASETLOADER_H_ */
