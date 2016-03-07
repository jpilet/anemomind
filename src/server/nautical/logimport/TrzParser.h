/*
 *  Created on: 2014-06-26
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TRZPARSER_H_
#define TRZPARSER_H_

#include <server/common/Hierarchy.h>

namespace sail {

class ParsedTrzLine {
 public:
  ParsedTrzLine(std::shared_ptr<HTree> tree_, std::string input_) :
    _tree(tree_), _input(input_) {}

  int childCount() const {
    return _tree->childCount();
  }

  ParsedTrzLine child(int index) const {
    return ParsedTrzLine(_tree->child(index), _input);
  }

  int index() const {
    return _tree->index();
  }

  std::string data() const {
    return _input.substr(_tree->left(), _tree->count());
  }

  bool isLeaf() const {
    return childCount() == 0;
  }

  bool empty() const {
    return !bool(_tree);
  }

  const std::shared_ptr<HTree> &tree() const {
    return _tree;
  }

  const int dataLength() const {
    return _tree->count();
  }
 private:
  std::shared_ptr<HTree> _tree;
  std::string _input;
};

class TrzParser {
 public:
  TrzParser();
  ParsedTrzLine parse(std::string line);
  Array<ParsedTrzLine> parseFile(std::istream &file);
  Array<ParsedTrzLine> parseFile(std::string filename);
  void disp(std::ostream *dst, const ParsedTrzLine &line, int depth = 0);
 private:
  Hierarchy _h;
};

/*
 * Exports to a format that easy to parse from matlab
 */
void exportToMatlab(std::string filename, Array<ParsedTrzLine> data);

}

#endif
