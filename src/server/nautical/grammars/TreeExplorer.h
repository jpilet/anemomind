/*
 *  Created on: 2014-06-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TREEVIEWER_H_
#define TREEVIEWER_H_

#include <server/common/Hierarchy.h>
#include <ostream>

namespace sail {

std::shared_ptr<HTree> exploreTree(Array<HNode> nodeinfo, std::shared_ptr<HTree> tree, std::ostream *out = nullptr,

    // An optional function that, given a node, returns a string with information to display about that node.
    std::function<std::string(std::shared_ptr<HTree>)> infoFun = std::function<std::string(std::shared_ptr<HTree>)>());

void outputLogGrammar(
    std::ostream *dst,
    const Array<HNode> &info,
        const std::shared_ptr<HTree> &tree,
        std::function<std::string(std::shared_ptr<HTree>)> infoFun);

} /* namespace sail */

#endif /* TREEVIEWER_H_ */
