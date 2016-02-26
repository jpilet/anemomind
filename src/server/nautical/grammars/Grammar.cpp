/*
 *  Created on: May 21, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/grammars/Grammar.h>

namespace sail {

namespace {
  Arrayi listNodeIndicesOfInterest(Array<HNode> nodes, std::string desc) {
    int count = nodes.size();
    Arrayi inds(count);
    int counter = 0;
    for (int i = 0; i < count; i++) {
      if (nodes[i].description() == desc) {
        inds[counter] = i;
        counter++;
      }
    }
    return inds.sliceTo(counter);
  }

  bool inSet(int index, Arrayi set) {
    for (auto x : set) {
      if (index == x) {
        return true;
      }
    }
    return false;
  }

  void markNodesRecursively(std::shared_ptr<HTree> tree,
      Arrayi inds, NavDataset navs,
      Arrayb dst) {
    if (inSet(tree->index(), inds)) {
      dst.slice(tree->left(), tree->right()).setTo(true);
    } else {
      Array<std::shared_ptr<HTree> > children = tree->children();
      for (auto c : children) {
        markNodesRecursively(c, inds, navs, dst);
      }
    }
  }
}

Arrayb markNavsByDesc(std::shared_ptr<HTree> tree,
    Array<HNode> nodeInfo, Array<Nav> allnavs,
    std::string label) {

    Arrayi inds = listNodeIndicesOfInterest(nodeInfo, label);
    if (inds.empty()) { // Probably a bug if inds are empty.
      return Arrayb();
    }

    int count = allnavs.size();
    Arrayb dst = Arrayb::fill(count, false);
    markNodesRecursively(tree, inds, NavCompat::fromNavs(allnavs), dst);
    return dst;
}


}

