/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef PROPORTIONATESAMPLER_H_
#define PROPORTIONATESAMPLER_H_

#include <server/common/Array.h>

namespace sail {

class ProportionateSampler {
 public:
  ProportionateSampler(Arrayd proportions);

  int get(double x) const;
  void remove(int index);
  int getAndRemove(double x);

  Arrayd proportions() const;
  Arrayb selected() const;
  Arrayb remaining() const;
 private:
  int _offset, _count;
  Arrayd _values;

  int getBySum(int node, double x) const;
  double fillInnerNodes(int root);
  static int parent(int index);
  static int leftChild(int index);
  static int rightChild(int index);

  bool isLeaf(int index) const {
    return _offset <= index;
  }
};

}

#endif /* PROPORTIONATESAMPLER_H_ */
