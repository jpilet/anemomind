/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef GRAMMAR_H_
#define GRAMMAR_H_

#include <server/common/Hierarchy.h>
#include <server/nautical/Nav.h>
#include <memory>
#include <server/math/hmm/StateAssign.h>

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
  virtual std::shared_ptr<HTree> parse(Array<Nav> navs,
      std::shared_ptr<StateAssign> hints = std::shared_ptr<StateAssign>()) = 0;

  /*
   * Returns an array of HNode
   */
  virtual Array<HNode> nodeInfo() = 0;

  virtual ~Grammar() {}
};


Arrayb markNavsByDesc(std::shared_ptr<HTree> tree,
    Array<HNode> nodeInfo, Array<Nav> allnavs,
    std::string label);

} /* namespace sail */

#endif /* GRAMMAR_H_ */
