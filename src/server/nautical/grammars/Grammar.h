/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef GRAMMAR_H_
#define GRAMMAR_H_

namespace sail {

#include <server/common/Hierarchy.h>

/*
 * Every HTree node has an index.
 * This index refers to a certain node type.
 */
class GrammarNodeInfo {
 public:
  GrammarNodeInfo() {}
  GrammarNodeInfo(std::string code, std::string desc) : _code(code), _description(desc) {}

  // Returns a text code to identify the node type.
  // This code can be used to refer to a record in a database
  // with more information about the node. For instance,
  std::string code() {return _code;}

  // This is an informal description of the node
  // that can be used for debugging purposes.
  std::string description() {return _description;}
 private:
  std::string _code, _description;
};

/*
 * This is an abstract class to
 * parse an array of Navs and return a parse tree.
 */
class Grammar {
public:
  /*
   * Should parse 'navs' using the underlying grammar
   * and return a parse tree for the result.
   */
  virtual std::shared_ptr<HTree> parse(Array<Nav> navs) = 0;

  /*
   * Returns an array of
   */
  virtual Array<GrammarNodeInfo> nodeInfo() = 0;

  virtual ~Grammar() {}
};

} /* namespace sail */

#endif /* GRAMMAR_H_ */
