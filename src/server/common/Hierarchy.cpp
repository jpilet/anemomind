/*
 *  Created on: 2014-01-29
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Hierarchy.h"

#include <sstream>
#include <assert.h>
#include <server/common/ArrayIO.h>
#include <server/common/logging.h>
#include <server/common/string.h>
#include <stdexcept>
#include <vector>
#include <server/common/ArrayBuilder.h>
#include <server/common/Functional.h>

namespace sail {

bool HNode::operator== (const HNode &other) const {
  return _index == other._index && _parent == other._parent
      && _description == other._description && _code == other._code;
}

HNode::HNode(int index, int parent, std::string code, std::string label)
    : _index(index), _parent(parent), _description(label), _code(code) {
}

namespace {
  std::string followDescs(bool isParent, int index,
      Array<HNode> nodes,
      Array<bool> isLeaf,
      Array<std::string> descriptions) {
      if (index == -1) {
        return "";
      } else {
        isLeaf[index] = !isParent;
        if (descriptions[index].empty()) {
          HNode node = nodes[index];
          descriptions[index] = followDescs(true, node.parent(),
              nodes, isLeaf, descriptions) + node.description();
        }
        return descriptions[index] + (isParent? "/" : "");
      }
  }
}

Array<std::string> HNode::makeDescriptionList(Array<HNode> nodes) {
  int count = nodes.size();
  Arrayb leaf = Arrayb::fill(count, false);
  Array<std::string> descriptions(count);
  for (int i = 0; i < nodes.size(); i++) {
    followDescs(false, i, nodes, leaf, descriptions);
  }
  return descriptions.slice(leaf);
}

namespace {
// Checks that
// if a node has a parent, that parent is defined.
void checkHNodeValidParents(Array<HNode> nodes) {
  int count = nodes.size();
  for (int i = 0; i < count; i++) {
    HNode &node = nodes[i];
    if (node.hasParent()) {
      assert(nodes[node.parent()].defined());
    }
  }
}

void checkMaxNodeInds(Array<HNode> nodes) {
  int count = nodes.size();
  for (int i = 0; i < count; i++) {
    CHECK_LT(nodes[i].index(), count);
  }
}

// Returns a new array where an HNode with index 'i' is located at position 'i'.
// Also makes sure that two nodes don't have the same index.
Array<HNode> arrangeHNodes(Array<HNode> nodes) {
  checkMaxNodeInds(nodes);
  int count = nodes.size();
  Array<HNode> dst(count);
  for (int i = 0; i < count; i++) {
    HNode &node = nodes[i];
    assert(dst[node.index()].undefined());
    dst[node.index()] = node;
  }
  return dst;
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
Array<Arrayi> listChildren(Array<HNode> nodes) {
  int count = nodes.size();
  Array<ArrayBuilder<int> > children(count);
  for (int i = 0; i < count; i++) {
    HNode &node = nodes[i];
    assert(node.index() == i);
    if (node.defined() && node.hasParent()) {
      children[node.parent()].add(node.index());
    }
  }
  return toArray(map(children, [&](ArrayBuilder<int> builder) {
    return builder.get();
  }));
}

// Sub-routine to calcLevelPerNode.
void traverseChildrenAndOutputLevel(int level, int currentNode, Array<Arrayi> childrenPerNode, Arrayi *levelPerNodeInOut) {
  assert(levelPerNodeInOut != nullptr);
  assert((*levelPerNodeInOut)[currentNode] == -1);
  (*levelPerNodeInOut)[currentNode] = level;

  int nextLevel = level + 1;
  Arrayi &children = childrenPerNode[currentNode];
  int childCount = children.size();
  for (int i = 0; i < childCount; i++) {
    traverseChildrenAndOutputLevel(nextLevel, children[i], childrenPerNode, levelPerNodeInOut);
  }
}

// Computes the level for each node in a routed tree.
Arrayi calcLevelPerNode(int rootNode, Array<Arrayi> children) {
  int count = children.size();
  Arrayi levelPerNode(count);
  levelPerNode.setTo(-1);
  traverseChildrenAndOutputLevel(0, rootNode, children, &levelPerNode);
  return levelPerNode;
}

Arrayb calcTerminals(Array<Arrayi> children) {
  return toArray(map(children, [&] (const Arrayi &c) {
    return c.empty();
  }));
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
    int level = levelPerNode[i];
    int p = -1;
    for (int lastNode = i; nodes[lastNode].hasParent(); lastNode = p) {
      p = nodes[lastNode].parent();
      level--;
      anc(i, level) = p;
    }
  }
  return anc;
}
}

std::shared_ptr<HTree> HTree::lastChild() {
  throw std::runtime_error("lastChild not defined for this object");
  return std::shared_ptr<HTree>();
}

Array<std::shared_ptr<HTree> > HTree::children() {
  int count = childCount();
  Array<std::shared_ptr<HTree> > dst(count);
  for (int i = 0; i < count; i++) {
    dst[i] = child(i);
  }
  return dst;
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

std::string getLabel(int index, Array<HNode> labels) {
  if (labels.empty()) {
    std::stringstream ss;
    ss << index;
    return ss.str();
  } else {
    HNode &h = labels[index];
    assert(h.index() == index); // Should be ordered.
    return h.description();
  }
}
}

void HInner::disp(std::ostream *out, Array<HNode> labels, int depth, int maxDepth) const {
  if (depth < maxDepth) {
    insertIndent(out, depth);
    *out << "HGeneral(" << getLabel(_index, labels) << ") in [" << left() << ", " << right() << "[" << std::endl;
    int count = _children.size();
    int nextIndent = depth + 1;
    for (int i = 0; i < count; i++) {
      _children[i]->disp(out, labels, nextIndent, maxDepth);
    }
  }
}

void HLeaves::disp(std::ostream *out, Array<HNode> labels, int depth, int maxDepth) const {
  if (depth < maxDepth) {
    insertIndent(out, depth);
    *out << "HTerminals(" << getLabel(_index, labels) << ") in [" << left() << ", " << right() << "[" << std::endl;
  }
}

bool HLeaves::equals(std::shared_ptr<HTree> other) const {
  return topEquals(other) && other->childCount() == 0;
}


Hierarchy::Hierarchy(Array<HNode> unorderedNodes) : _nodes(arrangeHNodes(unorderedNodes)) {
  checkHNodeValidParents(_nodes);
  _rootNode = getHRootNode(_nodes);
  assert(_rootNode != -1);
  Array<Arrayi> children = listChildren(_nodes);
  _childrenPerNode = children;
  _isTerminal = calcTerminals(children);
  _levelPerNode = calcLevelPerNode(_rootNode, children);
  _ancestors = makeAncestors(_nodes, _levelPerNode);
}

void Hierarchy::addTerminal(int left, std::shared_ptr<HTree> tree, int nodeIndex) const {
  assert(left == tree->right());
  assert(_isTerminal[nodeIndex]);
  bool wasAdded = addTerminalSub(tree, nodeIndex);
  assert(wasAdded);
}

bool Hierarchy::addTerminalSub(std::shared_ptr<HTree> tree, int nodeIndex) const {
  if (tree->index() == nodeIndex) {
    HLeaves *leaves = dynamic_cast<HLeaves*>(tree.get());
    assert(leaves != nullptr);
    leaves->add(nodeIndex);
    return true;
  } else {
    int treeIndex = tree->index();
    assert(treeIndex != -1);
    int treeLevel = _levelPerNode[treeIndex];
    if (_ancestors(nodeIndex, treeLevel) == treeIndex) { // if this node can be an ancestor of nodeIndex
      std::shared_ptr<HTree> lastChild = tree->lastChild();
      if (!addTerminalSub(lastChild, nodeIndex)) {
        std::shared_ptr<HTree> child = wrap(tree->right(), treeLevel+1, nodeIndex);
        assert(_nodes[child->index()].parent() == treeIndex);
        HInner *inner = dynamic_cast<HInner*>(tree.get());
        assert(inner != nullptr);
        inner->add(child);
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

HNode Hierarchy::getNodeByName(const std::string &name) const {
  for (auto n : _nodes) {
    if (n.description() == name) {
      return n;
    }
  }
  return HNode();
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
