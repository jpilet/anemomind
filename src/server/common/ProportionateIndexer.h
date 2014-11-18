/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef PROPORTIONATESAMPLER_H_
#define PROPORTIONATESAMPLER_H_

#include <server/common/Array.h>

namespace sail {

class ProportionateIndexer {
 public:
  ProportionateIndexer() : _offset(0), _count(0) {}
  ProportionateIndexer(Arrayd proportions);

  int get(double x) const;
  void remove(int index);
  int getAndRemove(double x);

  Arrayd proportions() const;
  Arrayb selected() const;
  Arrayb remaining() const;

  double sum() const {return _values[0];}
 private:
  int _offset, _count;
  Arrayd _values;

  double fillInnerNodes(int root);
  static int parent(int index);
  static int leftChild(int index);
  static int rightChild(int index);

  bool isLeaf(int index) const {
    return _offset <= index;
  }

  // For advanced use.
  class Result {
   public:
    Result(int index_, double localX_) :
      index(index_), localX(localX_) {}
    int index;
    double localX;
  };
  Result getBySum(int node, double x) const;
  Result getBySum(double x) const {return getBySum(0, x);}
};

}

#endif /* PROPORTIONATESAMPLER_H_ */
