/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_TILES_TILEUTILS_H_
#define SERVER_NAUTICAL_TILES_TILEUTILS_H_

#include <string>
#include <server/common/Array.h>
#include <server/nautical/Nav.h>

namespace sail {

class NavDataset;

void analyzeNavDataset(const std::string &dstFilename, const NavDataset &ds);

class TileGeneratorParameters;
void processTiles(const TileGeneratorParameters &params,
   std::string boatId, std::string navPath,
   std::string boatDat, std::string polarDat);

}

#endif
