/*
 *  Created on: 2014-01-29
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  This file models a very primitive form of grammar based on a hierarchy.
 *        http://en.wikipedia.org/wiki/Hierarchy
 *
 *  The idea is that the hierarchy consists of a set of HNode and each node has
 *  a parent. Nodes that have the same parent and occur together a grouped together.
 *  This is useful for grouping of states in a sail race and much more simple than
 *  a full-fledged context-free grammar.
 *
 *  Some important objects:
 *    HNode: Used to define the hierarchical grouping grammar. An array of HNode is required by the Hierarchy constructor.
 *    HTree: A general parse tree node returned from Hierarchy::parse
 *    Hierarchy: A grammar compiled from an array of HNode. It has a parse method.
 *
 */

#ifndef HIERARCHY_H_
#define HIERARCHY_H_

#include <string>
#include "MDArray.h"
#include <vector>
#include <memory>
#include <iosfwd>
#include <map>

namespace sail {

// Represents a node in the hierarchy. An array of HNode can define a grammar and is required by
// the constructor of the Hierarchy class.
class HNode {
 public:
  HNode() : _index(-1), _parent(-1) {}
  HNode(int index, int parent, std::string code, std::string description);

  bool defined() const {
    return _index != -1;
  }
  bool undefined() const {
    return !defined();
  }
  bool hasParent() const {
    return _parent != -1;
  }
  bool isRoot() const {
    return !hasParent();
  }
  const std::string &description() const {
    return _description;
  }
  const std::string &code() const {
    return _code;
  }
  int index() const {
    return _index;
  }
  int parent() const {
    return _parent;
  }

  bool operator== (const HNode &other) const;

  static Array<std::string> makeDescriptionList(Array<HNode> nodes);
 private:
  int _index, _parent;
  std::string _description, _code;
};

// The output of Hierarchy::parse is std::shared_ptr<HTree>
//
// The purpose of this class is to group together a section of
// a parsed sequence of terminals, e.g. the output from
// StateAssign.
class HTree {
 public:
  // Should return the left endpoint of this node in the parsed input.
  virtual int left() const = 0;

  // Returns the rightmost endpoint.
  virtual int right() const = 0;

  // Return the number of symbols captured by this node.
  virtual int count() const {
    return right() - left();
  }

  // Returns the index of the node.
  virtual int index() const = 0;

  virtual std::shared_ptr<HTree> lastChild();

  virtual ~HTree() {}

  virtual void disp(std::ostream *out, Array<HNode> labels = Array<HNode>(), int depth = 0, int maxDepth = 30) const = 0;

  virtual bool equals(std::shared_ptr<HTree> other) const = 0;

  virtual int childCount() const {
    return 0;
  }

  virtual std::shared_ptr<HTree> child(int index);

  Array<std::shared_ptr<HTree> > children();
 protected:
  bool topEquals(std::shared_ptr<HTree> other) const;
};

// This is a type of HTree that represents a section of terminals of the same type from
// the input string.
// This is less wasteful than allocating one HTree object of every single terminal.
class HLeaves : public HTree {
 public:
  HLeaves(int left, int index, int count = 1)
    : _index(index), _left(left), _count(count) {
    assert(index != -1);
  }
  int left() const {
    return _left;
  }
  int right() const {
    return _left + _count;
  }
  int count() const {
    return _count;
  }
  int index() const {
    return _index;
  }

  // nodeIndex should be _index. This is merely a redundant safety-feature.
  void add(int nodeIndex);
  void disp(std::ostream *out, Array<HNode> labels, int depth, int maxDepth) const;
  bool equals(std::shared_ptr<HTree> other) const;
 private:
  int _index, _left, _count;
};

// This is an HTree that groups HTree subnodes.
class HInner : public HTree {
 public:
  HInner(int index, std::shared_ptr<HTree> firstChild);
  int left() const;
  int right() const;
  int index() const {
    return _index;
  }
  void add(std::shared_ptr<HTree> child);
  std::shared_ptr<HTree> lastChild();
  void disp(std::ostream *out, Array<HNode> labels, int depth, int maxDepth) const;
  int childCount() const {
    return _children.size();
  }
  std::shared_ptr<HTree> child(int index) {
    return _children[index];
  }
  bool equals(std::shared_ptr<HTree> other) const;
 private:
  int _index;
  std::vector<std::shared_ptr<HTree> > _children;
};

// Models a hierarchy to define a grammar to parse a string.
class Hierarchy {
 public:
  // Creates a hierarchy from a set of nodes to define a grammar.
  // Every node except for the root node has a parent node.
  Hierarchy(Array<HNode> unorderedNodes);

  // This method takes an array of terminals (integers for which 'isTerminal' returns true)
  // returns a parse tree as specified by this grammar.
  // 'inputTerminalString' could for instance be the output of 'StateAssign::solve'.
  std::shared_ptr<HTree> parse(Arrayi inputTerminalString) const;





  ////////////////////////////////////////////////////////
  // THE METHODS HERE ARE PUBLIC TO FACILITATE TESTING
  // BUT LESS INTERESTING TO THE USER.

  // Returns the level of a node in a rooted tree
  int level(int nodeIndex) const {
    return _levelPerNode[nodeIndex];
  }

  // Returns index of the root node
  int rootNode() const {
    return _rootNode;
  }

  // Returns whether a node is a terminal, that is, it has no children.
  bool isTerminal(int nodeIndex) const {
    return _isTerminal[nodeIndex];
  }

  virtual ~Hierarchy();

  HNode node(int index) const {return _nodes[index];}
  Array<HNode> nodes() const {return _nodes;}
  Array<Arrayi> childrenPerNode() const {return _childrenPerNode;}
  Arrayi children(int nodeIndex) const {return _childrenPerNode[nodeIndex];}
  int nodeCount() const {return _nodes.size();}
  HNode getNodeByName(const std::string &name) const;
 private:
  // Creates an ancestor at 'level' for a node of type 'nodeIndex'
  std::shared_ptr<HTree> wrap(int left, int level, int nodeIndex) const;
  void addTerminal(int left, std::shared_ptr<HTree> tree, int nodeIndex) const;
  bool addTerminalSub(std::shared_ptr<HTree> tree, int nodeIndex) const;

  // All the nodes of the grammar stored here
  Array<HNode> _nodes;

  // Index of the root-node, the top-level node of the hierarchy.
  int _rootNode;

  // The corresponding level in a rooted tree for every node. The root has level 0.
  Array<int> _levelPerNode;

  // Boolean array where true means the corresponding node is a terminal
  Arrayb _isTerminal;

  // A 2d array where element _ancestors(i, j) is the index for the ancestor of node 'i' at level 'j' in the hierarchy.
  MDArray2i _ancestors;

  // Table that holds the child indices of every HNode
  Array<Arrayi> _childrenPerNode;
};

} /* namespace sail */

#endif /* HIERARCHY_H_ */
