/*
 *  Created on: 2014-06-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TREEVIEWER_H_
#define TREEVIEWER_H_

#include <server/common/Hierarchy.h>
#include <ostream>

namespace sail {

std::shared_ptr<HTree> exploreTree(Array<HNode> nodeinfo, std::shared_ptr<HTree> tree, std::ostream *out = nullptr);

} /* namespace sail */

#endif /* TREEVIEWER_H_ */
