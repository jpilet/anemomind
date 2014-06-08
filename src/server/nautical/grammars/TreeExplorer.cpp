/*
 *  Created on: 2014-06-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TreeExplorer.h"
#include <server/common/string.h>
#include <iostream>

namespace sail {

namespace {
  void outputIndentedLineNoBreak(const std::string &s, int depth) {
    indent(&std::cout, 3*depth);
    std::cout << s;
  }

  void outputIndentedLine(const std::string &s, int depth) {
    outputIndentedLineNoBreak(s, depth);
    std::cout << std::endl;
  }

  void viewTreeSub(Array<HNode> nodeinfo, std::shared_ptr<HTree> tree, int depth, bool shallow,
      const char *prefix = "") {
    if (!bool(tree)) {
      outputIndentedLine("Empty tree node\n", depth);
      return;
    } else {
      int index = tree->index();
      std::string type = nodeinfo[tree->index()].description();
      int chn = tree->childCount();

      int choice = -1;
      do {
        outputIndentedLine(stringFormat("%sNode of type %d (%s) with %d children", prefix, index, type.c_str(), chn), depth);
        if (shallow) {
          return;
        }
        outputIndentedLine(stringFormat("spanning Nav indices in [%d, %d[ at depth %d:", tree->left(), tree->right(), depth), depth);
        for (int i = 0; i < chn; i++) {
          std::string prefi = stringFormat("%d. ", i+1);
          viewTreeSub(nodeinfo, tree->child(i), depth+1, true, prefi.c_str());
        }
        outputIndentedLine("0. Back to level above", depth);
        outputIndentedLineNoBreak("Choice? ", depth);
        std::cin >> choice;
        if (1 <= choice && choice <= chn) {
          viewTreeSub(nodeinfo, tree->child(choice-1), depth+1, false);
        }
      } while (choice != 0);
    }
  }
}



void viewTree(Array<HNode> nodeinfo, std::shared_ptr<HTree> tree) {
  viewTreeSub(nodeinfo, tree, 0, false);
}

} /* namespace sail */
