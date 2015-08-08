/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_MATH_PIECEWISEPOLYNOMIALS_H_
#define SERVER_MATH_PIECEWISEPOLYNOMIALS_H_

#include <server/math/Integral1d.h>
#include <server/common/Span.h>
#include <server/math/QuadForm.h>

namespace sail {
namespace PiecewisePolynomials {




template <int N>
struct Piece {
 // The fitting cost, as a function
 // of coefficients
 QuadForm<N, 1> quadCost;

 // The span over which it is fitted
 Spani span;
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
class Joint {
 public:
  class Ptr {
   public:
    Ptr(int index, double increase) : _index(index), _increase(increase) {}

    bool operator<(const Ptr &other) const {
      if (_increase == other._increase) {
        return _index < other._index;
      }
      return _increase < other._increase;
    }

    int index() const {
      return _index;
    }
   private:
    int _index;
    double _increase;
  };

  Joint() : _left(-1), _middle(-1), _right(-1), _increase(NAN) {}

  Joint(int left, int middle, int right, Integral1d<LineFitQF> itg) :
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
  Integral1d<LineFitQF> _itg;
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

class Joints {
 public:
  Joints(Integral1d<LineFitQF> itg) {
    int jointCount = itg.size() - 1;
    _joints = Array<Joint>(jointCount + 2);
    for (int i = 0; i < jointCount; i++) {
      int index = i+1;
      Joint j(i, index, i+2, itg);
      _joints[index] = j;
      _ptrs.insert(j.ptr());
    }
  }

  void step() {
    auto jp = _ptrs.begin();
    Joint &joint = _joints[jp->index()];
    _ptrs.erase(jp);
    _joints[joint.left()].updateRight(joint.right(), &_ptrs);
    _joints[joint.right()].updateLeft(joint.left(), &_ptrs);
  }

 private:
  std::set<Joint::Ptr> _ptrs;
  Array<Joint> _joints;
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
void optimize(Arrayd X, Arrayd Y, int sampleCount, LineKM sampleToX) {
  Array<QuadForm<N, 1> > qfs = buildQfs<N>(X, Y, sampleCount, sampleToX);
  Integral1d<QuadForm<N, 1> > itg(qfs);

}









}}



#endif /* SERVER_MATH_PIECEWISEPOLYNOMIALS_H_ */
