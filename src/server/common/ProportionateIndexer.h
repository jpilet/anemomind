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
 * to an integer i in 0..(N-1) with an associated proportion p_i,
 * with
 *
 *  p_0 + ... + p_(N-1) = S
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

  void remove(int index);
  void assign(int index, double newValue);
  int getAndRemove(double x);

  Arrayd proportions() const;
  Arrayb selected() const;
  Arrayb remaining() const;

  double sum() const {return _values[0];}
  double sumTo(double fracIndex) const;

  double integrate(double from, double to) const {
    return sumTo(to) - sumTo(from);
  }

  class LookupResult {
   public:
    LookupResult(int index_, double localX_, double x_) :
      index(index_), localX(localX_), x(x_) {}

    int index;
    double localX;
    double x;
    double cumulativeLeft() const {return x - localX;}
  };
  LookupResult get(double x) const {return getAdvanced(0, x, x);}

  int size() const {
    return _count;
  }
 private:
  int _offset; // Index in the the _values array where the leaf nodes start.
  int _count; // Number of proportions.
  Arrayd _values;

  double fillInnerNodes(int root);
  static int parent(int index);
  static int leftChild(int index);
  static int rightChild(int index);
  LookupResult getAdvanced(int nodeIndex, double localX, double initX) const;

  bool isLeaf(int index) const {
    return _offset <= index;
  }

  Arrayd prepare(int count);
};

}

#endif /* PROPORTIONATESAMPLER_H_ */
