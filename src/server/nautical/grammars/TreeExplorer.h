/*
 *  Created on: 2014-06-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TREEVIEWER_H_
#define TREEVIEWER_H_

#include <server/common/Hierarchy.h>

namespace sail {

void viewTree(Array<HNode> nodeinfo, std::shared_ptr<HTree> tree);

} /* namespace sail */

#endif /* TREEVIEWER_H_ */
