/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/LineFitter.h>
#include <cassert>
#include <server/math/Integral1d.h>
#include <set>
#include <iostream>
#include <server/common/string.h>

namespace sail {

LineFitter::LineFitter(double lambda, int minimumEdgeCount) :
    _lambda(lambda), _minimumEdgeCount(minimumEdgeCount) {}

namespace {
  Array<LineFitQF> buildQfs(LineKM sampling, int sampleCount, Arrayd X, Arrayd Y) {
    Array<LineFitQF> qfs = Array<LineFitQF>::fill(sampleCount, LineFitQF(0));
    int count = X.size();
    assert(count == Y.size());
    for (int i = 0; i < count; i++) {
      int index = int(floor(sampling.inv(X[i])));
      qfs[index] += LineFitQF::fitLine(X[i], Y[i]);
    }
    return qfs;
  }

  class DiscontEnv;
  class Discont {
   public:
    Discont() : index(-1), leftNeigh(-1), rightNeigh(-1), _env(nullptr) {}
    Discont(DiscontEnv *env, int index_);
    void remove();

    // How much the data term of the objective
    // function will increase when this discontinuity is removed.
    double eval() const;

    bool isInner() const;
    int index, leftNeigh, rightNeigh;

    bool exists() const {
      return index != -1;
    }
   private:
    double valueWith() const;
    double valueWithout() const;
    DiscontEnv *_env;
  };

  class DiscontEnv {
   public:
    DiscontEnv(Integral1d<LineFitQF> itg_);

    Integral1d<LineFitQF> itg;
    Array<Discont> disconts;

    Array<LineFitter::LineSegment> buildSegments() const;
    int countDisconts() const;
  };

  DiscontEnv::DiscontEnv(Integral1d<LineFitQF> itg_) : itg(itg_) {
    int count = itg.size() + 1;
    disconts = Array<Discont>(count);
    for (int i = 0; i < count; i++) {
      disconts[i] = Discont(this, i);
    }
  }

  int DiscontEnv::countDisconts() const {
    int counter = 0;
    for (int index = 0; index != -1; index = disconts[index].rightNeigh) {
      counter++;
    }
    return counter;
  }

  Array<LineFitter::LineSegment> DiscontEnv::buildSegments() const {
    int discontCount = countDisconts();
    int segmentCount = discontCount - 1;
    Array<LineFitter::LineSegment> dst(segmentCount);
    int from = 0;
    for (int i = 0; i < segmentCount; i++) {
      int to = disconts[from].rightNeigh;
      dst[i] = LineFitter::LineSegment(Spani(from, to), itg.integrate(from, to));
      from = to;
    }
    return dst;
  }

  Discont::Discont(DiscontEnv *env, int index_) :
      index(index_), _env(env) {
    leftNeigh = (index_ == 0? -1 : index_-1);
    rightNeigh = (index_ == _env->disconts.size()-1? -1 : index_+1);
  }

  bool Discont::isInner() const {
    return leftNeigh != -1 && rightNeigh != -1;
  }

  double Discont::eval() const {
    assert(index != -1);
    assert(isInner());
    double dif = valueWithout() - valueWith();
    assert(dif >= 0);
    return dif;
  }

  double evalLineFit(const LineFitQF &other) {
    if (other.pElement(1, 1) < 1.0e-6) {
      return 0;
    } else {
      return other.evalOpt2x1();
    }
  }

  double Discont::valueWith() const {
    auto q0 = _env->itg.integrate(leftNeigh, index);
    auto q1 = _env->itg.integrate(index, rightNeigh);
    return evalLineFit(q0) + evalLineFit(q1);
  }

  double Discont::valueWithout() const {
    auto q = _env->itg.integrate(leftNeigh, rightNeigh);
    return evalLineFit(q);
  }

  void Discont::remove() {
    assert(isInner());
    assert(index != -1);
    assert(_env->disconts[leftNeigh].rightNeigh == index);
    assert(_env->disconts[rightNeigh].leftNeigh == index);
    _env->disconts[leftNeigh].rightNeigh = rightNeigh;
    _env->disconts[rightNeigh].leftNeigh = leftNeigh;
    index = -1;
    leftNeigh = -1;
    rightNeigh = -1;
  }

  class DiscontRef {
   public:
    DiscontRef(Discont *d) : _d(d) {}
    bool operator< (const DiscontRef &other) const {
      double e0 = _d->eval();
      double e1 = other._d->eval();
      if (e0 == e1) {
        return _d->index < other._d->index;
      }
      return e0 < e1;
    }
    Discont *get() {
      return _d;
    }
   private:
    Discont *_d;
  };

  void insert(std::set<DiscontRef> *dst, DiscontRef x) {
    assert(x.get()->index != -1 && x.get()->isInner());
    dst->insert(x);
  }

}

Array<LineFitter::LineSegment> LineFitter::detect(LineKM sampling, int sampleCount, Arrayd X, Arrayd Y) const {
  Array<LineFitQF> qfs = buildQfs(sampling, sampleCount, X, Y);
  Integral1d<LineFitQF> integral(qfs);

  for (int i = 0; i < integral.size(); i++) {
    auto q = integral.integrate(i, i+1);
  }

  DiscontEnv env(integral);
  std::set<DiscontRef> ordered;
  int totalDiscontCount = env.disconts.size();
  int innerDiscontCount = totalDiscontCount - 2; // exclude the two end points.
  for (int i = 1; i < totalDiscontCount-1; i++) {
    insert(&ordered, DiscontRef(env.disconts.ptr(i)));
  }

  assert(ordered.size() == innerDiscontCount);
  int maxRemoveCount = innerDiscontCount - _minimumEdgeCount;
  double objfValue = _lambda*innerDiscontCount;
  for (int i = 0; i < totalDiscontCount-1; i++) {
    objfValue += integral.integrate(i, i+1).evalOpt2x1();
  }

  for (int i = 0; i < maxRemoveCount; i++) {
    DiscontRef r = *ordered.begin();

    // Every discontinuity costs lambda, so
    // when we remove a discontinuity the regularization
    // term will decrease, whereas the data term will increase
    // because of worse fit to the data.
    double change = r.get()->eval() - _lambda;

    if (change < 0) { // Reduction in objective function value.
      Discont *d = r.get();
      DiscontRef left(env.disconts.ptr(d->leftNeigh));
      DiscontRef right(env.disconts.ptr(d->rightNeigh));
      if (left.get()->isInner()) {
        ordered.erase(left);
      }
      if (right.get()->isInner()) {
        ordered.erase(right);
      }
      ordered.erase(r);

      // This will update all the references in env.
      r.get()->remove();

      if (left.get()->isInner()) {
        insert(&ordered, left);
      }
      if (right.get()->isInner()) {
        insert(&ordered, right);
      }
      objfValue += change;
    } else {
      break;
    }
  }
  return env.buildSegments();
}


}
