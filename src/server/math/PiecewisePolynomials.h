/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *
 *  There is a single function in this header file that is interesting
 *  to the user of the code. It is the function with signature
 *
 *  template <int N>
 *    Array<Piece<N> > optimize(Arrayd X, Arrayd Y,
 *      int sampleCount, LineKM sampleToX, int segmentCount);
 *
 *  It will approximate a signal consisting of values Y indexed by values X using
 *  a set of piecewise polynomials, of type Piece<N>. Here, N is the number of coefficients
 *  used to represent the polynomial, so for example a quadratic polynomial a*x^2 + b*x + c has N = 3.
 *  The number of polynomials in the approximation is segmentCount. The X values are assumed
 *  to lie on an integer grid so that an index in the grid maps to an x value with sampleToX.
 *  The grid indices range from 0 to sampleCount-1.
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
#include <server/common/ArrayBuilder.h>

namespace sail {
namespace PiecewisePolynomials {

namespace INTERNAL {
  template <int N>
  QuadForm<N, 1> getReg() {
    static QuadForm<N, 1> reg = QuadForm<N, 1>::makeReg(1.0e-9);
    return reg;
  }

  template <int N>
  MDArray2d calcCoefs(QuadForm<N, 1> quad) {
    return (getReg<N>() + quad).minimize();
  }

  template <int N>
  double evalQF(const QuadForm<N, 1> &qf) {
    auto coefs = calcCoefs(qf);
    return qf.eval(coefs.ptr());
  }
}

// This is a piece in the optimal segmentation.
template <int N>
struct Piece {
  // The span over which it is fitted
  Spani span;

  // The fitting cost, as a function
  // of coefficients
  QuadForm<N, 1> quadCost;

  // If this pieces represents a constant value,
  // this function returns that value.
  double constantValue() const {
    static_assert(N == 1, "Only applicable to constants");
    return regularized().minimize1x1();
  }

  LineKM line() const {
    static_assert(N == 2, "Only applicable to lines");
    double mAndK[2] = {0, 0};
    regularized().minimize2x1(mAndK);
    return LineKM(mAndK[1], mAndK[0]);
  }

  QuadForm<N, 1> regularized() const {
    return quadCost + INTERNAL::getReg<N>();
  }

  double cost() const {
    return INTERNAL::evalQF<N>(quadCost);
  }
};

namespace INTERNAL {

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
            Ptr(const Joint<N> *ptr) : _ptr(ptr) {}

            // Used both to identify and order the pointer in the set.
            bool operator<(const Ptr &other) const {
              if (increase() == other.increase()) {
                return index() < other.index();
              }
              return increase() < other.increase();
            }

            int index() const {
              return _ptr->middle();
            }

            double increase() const {
              return _ptr->increase();
            }
           private:
            const Joint<N> *_ptr;
          };

          Joint() : _left(-1), _middle(-1), _right(-1), _increase(NAN) {}

          Joint(int left, int middle, int right, Integral1d<QuadForm<N, 1> > itg) :
            _itg(itg), _left(left), _middle(middle), _right(right) {
            computeIncrease();
          }

          bool defined() const {
            return _middle != -1;
          }

          Ptr ptr() const {
            assert(defined());
            return Ptr(this);
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

          int middle() const {
            return _middle;
          }

          double increase() const {
            return _increase;
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
            auto after =  evalFitness(_itg, _left, _right);
            auto before = evalFitness(_itg, _left, _middle) + evalFitness(_itg, _middle, _right);
            _increase = after - before;
            // We would like to check that the increase is non-negative with an assertion,
            // but that may be a bad idea due to cancellation effects making that number inaccurate.
            // the final result may despite this still be useful.
          }
        };




        // This is an intermediate result object that makes
        // it easy to obtain the pieces for various optimization
        // settings. It is constructed using the buildMerges method of
        // the Joints class.
        template <int N>
        class Merges {
         public:
          Merges(Integral1d<QuadForm<N, 1> > itg,
              Arrayd costs, Arrayi joints) :
            _itg(itg), _costs(costs), _joints(joints) {
            assert(_costs.size() == _joints.size() + 1);
          }

          // Given a set of joint indices, these are the
          Array<Piece<N> > getPiecesForJoints(Arrayi joints) const {
            Arrayi bds = getBoundingIndices(joints);
            int n = bds.size() - 1;
            Array<Piece<N> > dst(n);
            for (int i = 0; i < n; i++) {
              auto from = bds[i];
              auto to = bds[i+1];
              Spani span(from, to);
              dst[i] = Piece<N>{span, _itg.integrate(from, to)};
            }
            return dst;
          }

          // Get all the pieces from the joints of this object.
          Array<Piece<N> > pieces() const {
            return getPiecesForJoints(_joints);
          }
         private:
          Arrayi joints() const {
            return _joints;
          }

          Arrayi getBoundingIndices(Arrayi joints) const {
            auto n = _itg.size() + 1;
            auto marked = Arrayb::fill(n, true);
            for (auto index: joints) {
              marked[index] = false;
            }
            Arrayi bds(n);
            int counter = 0;
            for (int i = 0; i < n; i++) {
              if (marked[i]) {
                bds[counter] = i;
                counter++;
              }
            }
            return bds.sliceTo(counter);
          }

          Arrayi joints(int n) const {
            if (n < 0) {
              return Arrayi();
            } else if (_joints.size() <= n) {
              return _joints.sliceTo(n);
            }
            return _joints;
          }

          Arrayi jointsForSegmentCount(int segmentCount) {
            int remainingJointCount = segmentCount - 1;
            int pickedJointCount = totalJointCount() - remainingJointCount;
            return joints(pickedJointCount);
          }

          int totalJointCount() const {
            return _itg.size() - 1;
          }

          double costBefore(int pickedCount) const {
            return _costs[pickedCount];
          }

          double costAfter(int pickedCount) const {
            return _costs[pickedCount + 1];
          }

          Integral1d<QuadForm<N, 1> > _itg;
          Arrayd _costs;
          Arrayi _joints;
        };


        // Helper class.
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
              _ptrs.insert(_joints[index].ptr());
            }
          }

          int step() {
            auto jp0 = _ptrs.begin();
            auto jp = *jp0;
            Joint<N> &joint = _joints[jp.index()];
            _ptrs.erase(jp0);
            _joints[joint.left()].updateRight(joint.right(), &_ptrs);
            _joints[joint.right()].updateLeft(joint.left(), &_ptrs);
            _cost += jp.increase();
            return jp.index();
          }

          Merges<N> buildMerges(int remainingJointCount = 0,
              double maxCost = std::numeric_limits<double>::infinity()) {
            assert(_ptrs.size() + 1 == _itg.size());
            ArrayBuilder<double> costs;
            ArrayBuilder<int> joints;
            costs.add(_cost);
            while (_cost < maxCost && _ptrs.size() > remainingJointCount) {
              joints.add(step());
              costs.add(_cost);
            }
            return Merges<N>(_itg, costs.get(), joints.get());
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
}

/*
 *
 *
 *
 *
 *
 * The main function
 * to fit piecewise polynomials
 * to observations.
 *
 * Inspired by the SimplifyCurve algorithm.
 *
 *
 *
 */
template <int N>
Array<Piece<N> > optimizeForSegmentCount(Arrayd X, Arrayd Y,
    int sampleCount, LineKM sampleToX, int segmentCount) {
  using namespace INTERNAL;
  Integral1d<QuadForm<N, 1> > itg(buildQfs<N>(X, Y, sampleCount, sampleToX));
  Joints<N> joints(itg);
  int remainingJointCount = segmentCount-1;
  auto merges = joints.buildMerges(remainingJointCount);
  return merges.pieces();
}

template <int N>
Array<Piece<N> > optimizeForMaxCost(Arrayd X, Arrayd Y,
    int sampleCount, LineKM sampleToX, double maxCost) {
  using namespace INTERNAL;
  Integral1d<QuadForm<N, 1> > itg(buildQfs<N>(X, Y, sampleCount, sampleToX));
  Joints<N> joints(itg);
  auto merges = joints.buildMerges(0, maxCost);
  return merges.pieces();
}








}}



#endif /* SERVER_MATH_PIECEWISEPOLYNOMIALS_H_ */
