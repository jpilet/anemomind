/*
 *  Created on: 2014-06-18
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef HNODEDSL_H_
#define HNODEDSL_H_

#include <server/common/Array.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/Hierarchy.h>

namespace sail {

class HNodeGroup {
 public:
  // For creating an inner grouping node
  HNodeGroup(std::string description, ArrayBuilder<HNodeGroup> subnodes);

  // For creating an inner grouping node with explicitly assigning an index
  HNodeGroup(int index, std::string description, ArrayBuilder<HNodeGroup> subnodes);

  // For creating a leav nodes
  HNodeGroup(int index, std::string desc);
  Array<HNode> compile(const char *codeFormat = "UnnamedHNodes-%03d");
 private:
  int _index;
  std::string _desc;

  Array<HNodeGroup> _children;
  int getMaxIndex();
  int checkIndexedAndCountNodes(Arrayb marked);
  int fillNodes(Array<HNode> nodes, int counter, int parent, const char *fmt);
};

ArrayBuilder<HNodeGroup> operator+(HNodeGroup a, HNodeGroup b);
ArrayBuilder<HNodeGroup> operator+(ArrayBuilder<HNodeGroup> a, HNodeGroup b);
ArrayBuilder<HNodeGroup> operator+(HNodeGroup a, ArrayBuilder<HNodeGroup> b);

} /* namespace sail */

#endif /* HNODEDSL_H_ */
