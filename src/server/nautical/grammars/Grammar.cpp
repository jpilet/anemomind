/*
 *  Created on: May 21, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/grammars/Grammar.h>

#include <server/common/ArrayBuilder.h>
#include <server/nautical/NavCompatibility.h>

namespace sail {

using NavCompat::timeAt;

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
      Arrayi inds,
      const NavDataset& navs,
      ArrayBuilder<std::pair<TimeStamp, TimeStamp>>* dst) {
    if (inSet(tree->index(), inds)) {
      dst->add(std::make_pair(timeAt(navs, tree->left()),
                              timeAt(navs, tree->right() - 1)));
    } else {
      Array<std::shared_ptr<HTree> > children = tree->children();
      for (auto c : children) {
        markNodesRecursively(c, inds, navs, dst);
      }
    }
  }
}

Array<std::pair<TimeStamp, TimeStamp>> markNavsByDesc(std::shared_ptr<HTree> tree,
    Array<HNode> nodeInfo, const NavDataset& navs,
    std::string label) {

    Arrayi inds = listNodeIndicesOfInterest(nodeInfo, label);
    if (inds.empty()) { // Probably a bug if inds are empty.
      return Array<std::pair<TimeStamp, TimeStamp>>();
    }
    ArrayBuilder<std::pair<TimeStamp, TimeStamp>> result;
    markNodesRecursively(tree, inds, navs, &result);
    return result.get();
}


}

