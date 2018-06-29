/*
 *  Created on: 2014-06-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "TreeExplorer.h"
#include <server/common/string.h>
#include <iostream>

namespace sail {

namespace {
  void outputIndentedLineNoBreak(const std::string &s, int depth, std::ostream *out) {
    indent(out, 3*depth);
    *out << s;
  }

  void outputIndentedLine(const std::string &s, int depth, std::ostream *out) {
    outputIndentedLineNoBreak(s, depth, out);
    *out << std::endl;
  }

  std::shared_ptr<HTree> exploreTreeSub(Array<HNode> nodeinfo, std::shared_ptr<HTree> tree, int depth, bool shallow,
      const char *prefix, std::ostream *out, std::function<std::string(std::shared_ptr<HTree>)> infoFun) {
    if (!bool(tree)) {
      outputIndentedLine("Empty tree node\n", depth, out);
    } else {
      int index = tree->index();
      std::string type = nodeinfo[tree->index()].description();
      int chn = tree->childCount();

      int choice = -1;
      do {
        std::string infostr;
        if (bool(infoFun)) {
          infostr = " (" + infoFun(tree) + ")";
        }
        outputIndentedLine(stringFormat("%s %s (%d children) %s count %d",
            prefix, type.c_str(), chn, infostr.c_str(),
            tree->right() - tree->left()),
            depth, out);
        if (shallow) {
          return std::shared_ptr<HTree>();
        }
        outputIndentedLine(stringFormat("spanning Nav indices in [%d, %d[ at depth %d:",
            tree->left(), tree->right(), depth),
            depth, out);
        for (int i = 0; i < chn; i++) {
          std::string prefi = stringFormat("%d. ", i+1);
          exploreTreeSub(nodeinfo, tree->child(i), depth+1, true, prefi.c_str(), out, infoFun);
        }
        outputIndentedLine("0. Back to parent node", depth, out);
        outputIndentedLine("-1. Select this node to return", depth, out);
        outputIndentedLineNoBreak("Choice? ", depth, out);
        std::cin >> choice;
        if (1 <= choice && choice <= chn) {
          *out << "\n\n";
          std::shared_ptr<HTree> ret = exploreTreeSub(nodeinfo,
              tree->child(choice-1), depth+1, false, "", out, infoFun);
          *out << "\n\n";
          if (bool(ret)) {
            return ret;
          }
        }
      } while (choice != 0 && choice != -1);
      if (choice == -1) {
        return tree;
      }
    }
    return std::shared_ptr<HTree>();
  }
}



std::shared_ptr<HTree> exploreTree(Array<HNode> nodeinfo, std::shared_ptr<HTree> tree, std::ostream *out,
    std::function<std::string(std::shared_ptr<HTree>)> infoFun) {
  if (out == nullptr) {
    out = &(std::cout);
  }
  return exploreTreeSub(nodeinfo, tree, 0, false, "", out, infoFun);
}

void outputLogGrammarSub(std::ostream *dst,
    const Array<HNode> &info,
    const std::shared_ptr<HTree> &tree,
    std::function<std::string(std::shared_ptr<HTree>)> infoFun,
    int depth) {
  outputIndentedLine(info[tree->index()].description()
      + ": " + infoFun(tree), depth, dst);
  int deeper = 1 + depth;
  for (auto c: tree->children()) {
    outputLogGrammarSub(dst, info, c, infoFun, deeper);
  }
}

void outputLogGrammar(
    std::ostream *dst,
    const Array<HNode> &info,
        const std::shared_ptr<HTree> &tree,
        std::function<std::string(std::shared_ptr<HTree>)> infoFun) {
  outputLogGrammarSub(dst, info, tree, infoFun, 0);
}

} /* namespace sail */
