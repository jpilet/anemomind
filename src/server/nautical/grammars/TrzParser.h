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
  ParsedTrzLine(std::shared_ptr<HTree> tree_, std::string input_) : _input(input_), _tree(tree_) {}
  std::shared_ptr<HTree> tree() const {
    return _tree;
  }

  std::string input() const {
    return _input;
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
  void disp(std::ostream *dst, const ParsedTrzLine &line);
 private:
  void disp(std::ostream *dst, std::shared_ptr<HTree> tree, const std::string &s, int indent);
  Hierarchy _h;
  Array<Arrayi> _prec;
};

}

#endif
