/*
 *  Created on: 2014-01-29
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Hierarchy.h"
#include <assert.h>
#include <vector>
#include "ArrayIO.h"
#include "common.h"
#include  <sstream>

namespace sail
{

HNode::HNode(int index, int parent, std::string label) : _index(index), _parent(parent), _label(label) {
}

HNode HNode::makeRoot(int index, std::string label) {
  return HNode(index, -1, label);
}


namespace {
  // Checks that
  //  (i)  the index of every node corresponds to its location in the array
  //  (ii) if a node has a parent, that parent is defined.
  void checkHNodeArray(Array<HNode> nodes) {
    int count = nodes.size();
    for (int i = 0; i < count; i++) {
      HNode &node = nodes[i];
      assert(node.index() == i  || node.undefined());
      if (node.hasParent()) {
        assert(nodes[node.parent()].defined());
      }
    }
  }

  // Returns true iff all nodes that are defined have a parent
  bool haveParents(Array<HNode> nodes) {
    int count = nodes.size();
    for (int i = 0; i < count; i++) {
      HNode &node = nodes[i];
      if (node.defined() && !node.hasParent()) {
        return false;
      }
    }
    return true;
  }

  // Returns the root node of the hierarchy. There can only be one.
  int getHRootNode(Array<HNode> nodes) {
    int count = nodes.size();
    for (int i = 0; i < count; i++) {
      HNode &node = nodes[i];
      if (node.defined() && node.isRoot()) {
        assert(haveParents(nodes.sliceFrom(i+1)));
        return i;
      }
    }
    return -1;
  }

  // Builds a table that for every node lists the nodes for whom it is the parent.
  Array<std::vector<int> > listChildren(Array<HNode> nodes) {
    int count = nodes.size();
    Array<std::vector<int> > children(count);
    for (int i = 0; i < count; i++) {
      HNode &node = nodes[i];
      assert(node.index() == i);
      if (node.defined() && node.hasParent()) {
        children[node.parent()].push_back(node.index());
      }
    }
    return children;
  }

  // Sub-routine to calcLevelPerNode.
  void traverseChildrenAndOutputLevel(int level, int currentNode, Array<std::vector<int> > childrenPerNode, Arrayi *levelPerNodeInOut) {
    assert(levelPerNodeInOut != nullptr);
    Arrayi &lpn = *levelPerNodeInOut;
    assert(lpn[currentNode] == -1);
    lpn[currentNode] = level;
    int nextLevel = level + 1;
    std::vector<int> &children = childrenPerNode[currentNode];
    int childCount = children.size();
    for (int i = 0; i < childCount; i++) {
      traverseChildrenAndOutputLevel(nextLevel, children[i], childrenPerNode, levelPerNodeInOut);
    }
  }

  // Computes the level for each node in a routed tree.
  Arrayi calcLevelPerNode(int rootNode, Array<std::vector<int> > children) {
    int count = children.size();
    Arrayi levelPerNode(count);
    levelPerNode.setTo(-1);
    traverseChildrenAndOutputLevel(0, rootNode, children, &levelPerNode);
    return levelPerNode;
  }

  Arrayb calcTerminals(Array<std::vector<int> > children) {
    return children.map<bool>([&] (const std::vector<int> &c) {return c.empty();});
  }

  int getMaxLevel(Arrayi levels) {
    int maxl = 0;
    int count = levels.size();
    for (int i = 0; i < count; i++) {
      maxl = std::max(maxl, levels[i]);
    }
    return maxl;
  }

  MDArray2i makeAncestors(Array<HNode> nodes, Arrayi levelPerNode) {
    int maxLevel = getMaxLevel(levelPerNode);
    int count = nodes.size();
    assert(count == levelPerNode.size());
    MDArray2i anc(count, maxLevel+1);
    anc.setAll(-1);
    for (int i = 0; i < count; i++) {
      int lastNode = i;
      int level = levelPerNode[i];
      while (nodes[lastNode].hasParent()) {
        int p = nodes[lastNode].parent();
        level--;
        anc(i, level) = p;
        lastNode = p;
      }
    }
    return anc;
  }

  Array<std::string> listLabels(Array<HNode> nodes) {
    return nodes.map<std::string>([&] (const HNode &node) {return node.label();});
  }
}

void HTree::add(int nodeIndex) {
  throw std::runtime_error("This method is not defined for this object");
}

void HTree::add(std::shared_ptr<HTree> child) {
  throw std::runtime_error("This method is not defined for this object");
}

std::shared_ptr<HTree> HTree::lastChild() {
  throw std::runtime_error("lastChild not defined for this object");
  return std::shared_ptr<HTree>();
}

std::shared_ptr<HTree> HTree::child(int index) {
  throw std::runtime_error("child not defined for this object");
  return std::shared_ptr<HTree>();
}

bool HTree::topEquals(std::shared_ptr<HTree> other) const {
  return index() == other->index() && left() == other->left() && count() == other->count();
}

void HLeaves::add(int nodeIndex) {
  assert(nodeIndex == _index);
  _count++;
}

HInner::HInner(int index, std::shared_ptr<HTree> firstChild) : _index(index) {
  assert(index != -1);
  _children.push_back(firstChild);
}

int HInner::left() const {
  return _children.front()->left();
}

int HInner::right() const {
  return _children.back()->right();
}

void HInner::add(std::shared_ptr<HTree> child) {
  _children.push_back(child);
}

std::shared_ptr<HTree> HInner::lastChild() {
  return _children.back();
}

bool HInner::equals(std::shared_ptr<HTree> other) const {
  int cc = childCount();
  if (topEquals(other) && cc == other->childCount()) {
    for (int i = 0; i < cc; i++) {
      if (!_children[i]->equals(other->child(i))) {
        return false;
      }
    }
    return true;
  }
  return false;
}


namespace {
  void insertIndent(std::ostream *out, int count, int size = 2) {
    int len = count*size;
    for (int i = 0; i < len; i++) {
      *out << " ";
    }
  }

  std::string getLabel(int index, Array<std::string> labels) {
    if (labels.empty()) {
      std::stringstream ss;
      ss << index;
      return ss.str();
    } else {
      return labels[index];
    }
  }
}

void HInner::disp(std::ostream *out, Array<std::string> labels, int indent) const {
  insertIndent(out, indent);
  *out << "HGeneral(" << getLabel(_index, labels) << ") in [" << left() << ", " << right() << "[" << std::endl;
  int count = _children.size();
  int nextIndent = indent + 1;
  for (int i = 0; i < count; i++) {
    _children[i]->disp(out, labels, nextIndent);
  }
}

void HLeaves::disp(std::ostream *out, Array<std::string> labels, int indent) const {
  insertIndent(out, indent);
  *out << "HTerminals(" << getLabel(_index, labels) << ") in [" << left() << ", " << right() << "[" << std::endl;
}

bool HLeaves::equals(std::shared_ptr<HTree> other) const {
  return topEquals(other) && other->childCount() == 0;
}


Hierarchy::Hierarchy(Array<HNode> nodes) : _nodes(nodes) {
  checkHNodeArray(nodes);
  _rootNode = getHRootNode(nodes);
  assert(_rootNode != -1);
  Array<std::vector<int> > children = listChildren(nodes);
  _isTerminal = calcTerminals(children);
  _levelPerNode = calcLevelPerNode(_rootNode, children);
  _ancestors = makeAncestors(nodes, _levelPerNode);
  _labels = listLabels(nodes);
}

void Hierarchy::addTerminal(int left, std::shared_ptr<HTree> tree, int nodeIndex) const {
  assert(left == tree->right());
  assert(_isTerminal[nodeIndex]);
  assert(addTerminalSub(tree, nodeIndex));
}

bool Hierarchy::addTerminalSub(std::shared_ptr<HTree> tree, int nodeIndex) const {
  if (tree->index() == nodeIndex) {
    tree->add(nodeIndex);
    return true;
  } else {
    int treeIndex = tree->index();
    assert(treeIndex != -1);
    int treeLevel = _levelPerNode[treeIndex];
    if (_ancestors(nodeIndex, treeLevel) == tree->index()) { // if this node can be an ancestor of nodeIndex
      std::shared_ptr<HTree> lastChild = tree->lastChild();
      if (!addTerminalSub(lastChild, nodeIndex)) {
        tree->add(wrap(tree->right(), treeLevel+1, nodeIndex));
      }
      return true;
    } else {
      return false;
    }
  }
}

std::shared_ptr<HTree> Hierarchy::parse(Arrayi inds) const {
  assert(inds.hasData());
  std::shared_ptr<HTree> tree = wrap(0, 0, inds[0]);
  int count = inds.size();
  for (int i = 1; i < count; i++) {
    addTerminal(i, tree, inds[i]);
  }
  assert(tree->left() == 0);
  assert(tree->count() == count);
  return tree;
}

std::shared_ptr<HTree> Hierarchy::wrap(int left, int level, int nodeIndex) const {
  int dstLevel = _levelPerNode[nodeIndex];
  if (level == dstLevel) {
    assert(_isTerminal[nodeIndex]);
    return std::shared_ptr<HTree>(new HLeaves(left, nodeIndex));
  } else {
    assert(level < dstLevel);
    int anc = _ancestors(nodeIndex, level);
    assert(anc != -1);
    return std::shared_ptr<HTree>(new HInner(anc, wrap(left, level+1, nodeIndex)));
  }
}

Hierarchy::~Hierarchy() {
}

} /* namespace sail */
