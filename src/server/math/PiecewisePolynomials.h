/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_MATH_PIECEWISEPOLYNOMIALS_H_
#define SERVER_MATH_PIECEWISEPOLYNOMIALS_H_

#include <server/math/Integral1d.h>
#include <server/common/Span.h>
#include <server/math/QuadForm.h>
#include <server/common/LineKM.h>
#include <set>

#include <server/common/string.h>
#include <iostream>
#include <server/common/ArrayIO.h>

namespace sail {
namespace PiecewisePolynomials {




template <int N>
struct Piece {
  // The span over which it is fitted
  Spani span;

  // The fitting cost, as a function
  // of coefficients
  QuadForm<N, 1> quadCost;

  double constantValue() const {
    static_assert(N == 1, "Only applicable to constants");
    return quadCost.minimize1x1();
  }
};

template <int N>
MDArray2d calcCoefs(QuadForm<N, 1> quad) {
  static QuadForm<N, 1> reg = QuadForm<N, 1>::makeReg(1.0e-9);
  return (reg + quad).minimize();
}

template <int N>
double evalQF(const QuadForm<N, 1> &qf) {
  auto coefs = calcCoefs(qf);
  return qf.eval(coefs.ptr());
}

template <int N>
double evalFitness(Integral1d<QuadForm<N, 1> > itg, int from, int to) {
  return evalQF(itg.integrate(from, to));
}

// The point between two segments
template <int N>
class Joint {
 public:
  class Ptr {
   public:
    Ptr(int index, double increase) : _index(index), _increase(increase) {}

    // Used both to identify and order the pointer in the set.
    bool operator<(const Ptr &other) const {
      if (_increase == other._increase) {
        return _index < other._index;
      }
      return _increase < other._increase;
    }

    int index() const {
      return _index;
    }

    double increase() const {
      return _increase;
    }
   private:
    int _index;
    double _increase;
  };

  Joint() : _left(-1), _middle(-1), _right(-1), _increase(NAN) {}

  Joint(int left, int middle, int right, Integral1d<QuadForm<N, 1> > itg) :
    _left(left), _middle(middle), _right(right), _itg(itg) {
    computeIncrease();
  }

  bool defined() const {
    return _middle != -1;
  }

  Ptr ptr() const {
    assert(defined());
    return Ptr(_middle, _increase);
  }

  void updateLeft(int left, std::set<Joint::Ptr> *ptrs) {
    update(left, _right, ptrs);
  }

  void updateRight(int right, std::set<Joint::Ptr> *ptrs) {
    update(_left, right, ptrs);
  }

  int left() const {
    return _left;
  }

  int right() const {
    return _right;
  }
 private:
  Integral1d<QuadForm<N, 1> > _itg;
  int _left, _middle, _right;
  double _increase;

  void update(int left, int right, std::set<Joint::Ptr> *ptrs) {
    if (defined()) {
      ptrs->erase(ptr());
      _left = left;
      _right = right;
      computeIncrease();
      ptrs->insert(ptr());
    }
  }

  void computeIncrease() {
    // This is the increase in the total line fitness cost
    // when we approximate the signal between 'left' and 'right'
    // with a single straight line instead of two straight lines
    // divided at 'middle'
    _increase = evalFitness(_itg, _left, _right)
        - evalFitness(_itg, _left, _middle) - evalFitness(_itg, _middle, _right);

    // For a coarser approximation, we always expect an increase.
    assert(0 <= _increase);
  }
};


template <int N>
class Joints {
 public:
  typedef typename Joint<N>::Ptr JPtr;

  Joints(Integral1d<QuadForm<N, 1> > itg): _itg(itg) {
    _cost = 0;
    for (int i = 0; i < itg.size(); i++) {
      _cost += evalQF(itg.integrate(i, i+1));
    }
    int jointCount = itg.size() - 1;
    _joints = Array<Joint<N> >(jointCount + 2);
    for (int i = 0; i < jointCount; i++) {
      int index = i+1;
      Joint<N> j(i, index, i+2, itg);
      _joints[index] = j;
      _ptrs.insert(j.ptr());
    }
  }

  void step() {
    auto jp0 = _ptrs.begin();
    auto jp = *jp0;
    Joint<N> &joint = _joints[jp.index()];
    _ptrs.erase(jp0);
    _joints[joint.left()].updateRight(joint.right(), &_ptrs);
    _joints[joint.right()].updateLeft(joint.left(), &_ptrs);
    _cost += jp.increase();
  }

  bool empty() const {
    return _ptrs.empty();
  }

  void stepToJointCount(int jc) {
    while (_ptrs.size() > jc) {
      step();
    }
  }

  void stepToSegmentCount(int sc) {
    stepToJointCount(sc - 1);
  }

  Array<int> getBoundingSampleIndices() {
    Arrayi allIndices(2 + _ptrs.size());
    allIndices[0] = 0;
    allIndices[1] = _itg.size();
    int counter = 2;
    while (!_ptrs.empty()) {
      auto jp = _ptrs.begin();
      allIndices[counter] = jp->index();
      counter++;
      _ptrs.erase(jp);
    }
    assert(counter == allIndices.size());
    std::sort(allIndices.begin(), allIndices.end());
    return allIndices;
  }
 private:
  Integral1d<QuadForm<N, 1> > _itg;
  double _cost;
  std::set<JPtr> _ptrs;
  Array<Joint<N> > _joints;
};



template <int N>
Array<QuadForm<N, 1> > buildQfs(Arrayd X, Arrayd Y, int sampleCount, LineKM sampleToX) {
  int n = X.size();
  assert(n == Y.size());
  int segmentCount = sampleCount - 1;
  LineKM xToSample = sampleToX.makeInvFun();
  Array<QuadForm<N, 1> > qfs(segmentCount);
  for (int i = 0; i < n; i++) {
    int index = int(floor(xToSample(X[i])));
    if (0 <= index && index < segmentCount) {
      qfs[index] += QuadForm<N, 1>::fitPolynomial(X[i], Y[i]);
    }
  }
  return qfs;
}

template <int N>
Array<Piece<N> > optimize(Arrayd X, Arrayd Y,
    int sampleCount, LineKM sampleToX, int segmentCount) {
  Integral1d<QuadForm<N, 1> > itg(buildQfs<N>(X, Y, sampleCount, sampleToX));
  Joints<N> joints(itg);
  joints.stepToSegmentCount(segmentCount);
  auto bds = joints.getBoundingSampleIndices();
  return Spani(0, segmentCount).map<Piece<N> >([&](int segmentIndex) {
    int from = bds[segmentIndex];
    int to = bds[segmentIndex + 1];
    return Piece<N>{Spani(from, to), itg.integrate(from, to)};
  });
}









}}



#endif /* SERVER_MATH_PIECEWISEPOLYNOMIALS_H_ */
