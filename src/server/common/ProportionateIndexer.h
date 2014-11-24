/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef PROPORTIONATESAMPLER_H_
#define PROPORTIONATESAMPLER_H_

#include <server/common/Array.h>

namespace sail {

/*
 * A function that maps a real number in an interval [0, S[
 * to an integer i in 1..N with an associated proportion p_i,
 * with
 *
 *  p_1 + ... + p_N = S
 *
 * If we randomize a number x according to a uniform distribution [0, S[,
 * the probability that it will map to integer i is p_i/S.
 */
class ProportionateIndexer {
 public:
  ProportionateIndexer() : _offset(0), _count(0) {}
  ProportionateIndexer(Arrayd proportions);
  ProportionateIndexer(int count,
      std::function<double(int)> widthPerProp);

  int get(double x) const;
  void remove(int index);
  void assign(int index, double newValue);
  int getAndRemove(double x);

  Arrayd proportions() const;
  Arrayb selected() const;
  Arrayb remaining() const;

  double sum() const {return _values[0];}

  // For advanced use.
  class LookupResult {
   public:
    LookupResult(int index_, double localX_) :
      index(index_), localX(localX_) {}
    int index;
    double localX;
  };
  LookupResult getBySum(int node, double x) const;
  LookupResult getBySum(double x) const {return getBySum(0, x);}
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

  Arrayd prepare(int count);
};

}

#endif /* PROPORTIONATESAMPLER_H_ */
