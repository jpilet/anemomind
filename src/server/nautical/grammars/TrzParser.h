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
  ParsedTrzLine(std::shared_ptr<HTree> tree, std::string input) : _input(input), _tree(tree) {}
 private:
  std::shared_ptr<HTree> _tree;
  std::string _input;
};

class TrzParser {
 public:
 private:

};

}

#endif
