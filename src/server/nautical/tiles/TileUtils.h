/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_TILES_TILEUTILS_H_
#define SERVER_NAUTICAL_TILES_TILEUTILS_H_

#include <server/common/Array.h>
#include <server/nautical/NavDataset.h>
#include <server/nautical/filters/SmoothGpsFilter.h>
#include <server/nautical/grammars/WindOrientedGrammar.h>
#include <string>

namespace sail {

NavDataset filterNavs(
    const NavDataset& navs,
    DOM::Node *dst,
    const GpsFilterSettings& settings);

Array<NavDataset> extractAll(std::string description, NavDataset rawNavs,
                             const WindOrientedGrammar& grammar,
                             const std::shared_ptr<HTree>& tree);

}  // namespace sail

#endif
