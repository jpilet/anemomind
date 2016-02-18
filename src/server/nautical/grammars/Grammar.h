/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef GRAMMAR_H_
#define GRAMMAR_H_

#include <server/common/Hierarchy.h>
#include <server/nautical/Nav.h>
#include <server/nautical/grammars/UserHint.h>
#include <memory>

namespace sail {

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
  virtual std::shared_ptr<HTree> parse(NavCollection navs,
      Array<UserHint> hints = Array<UserHint>()) = 0;

  /*
   * Returns an array of HNode
   */
  virtual Array<HNode> nodeInfo() const = 0;

  virtual ~Grammar() {}

  virtual MDArray2b startOfRaceTransitions() const {return MDArray2b();};
  virtual MDArray2b endOfRaceTransitions() const {return MDArray2b();};
};


Arrayb markNavsByDesc(std::shared_ptr<HTree> tree,
    Array<HNode> nodeInfo, NavCollection allnavs,
    std::string label);

} /* namespace sail */

#endif /* GRAMMAR_H_ */
