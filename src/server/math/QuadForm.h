/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef QUADFUN_H_
#define QUADFUN_H_

#include <server/common/math.h>
#include <server/common/SymmetricMatrix.h>
#include <algorithm>
#include <server/math/LUImpl.h>
#include <cassert>
#include <server/common/LineKM.h>
#include <Eigen/Dense>
#include <array>
#include <server/common/logging.h>

namespace sail {

template <typename T, int srcRows, int srcCols>
Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>
  subMatrixByInds(
      const Eigen::Matrix<T, srcRows, srcCols>& src,
      const Array<int>& rowInds,
      const Array<int>& colInds) {
  int rowCount = rowInds.size();
  int colCount = colInds.size();
  Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> dst(
      rowCount, colCount);
  for (int i = 0; i < rowCount; i++) {
    for (int j = 0; j < colCount; j++) {
      dst(i, j) = src(rowInds[i], colInds[j]);
    }
  }
  return dst;
}

template <int N>
class IndexSplit {
public:
  IndexSplit(const std::array<bool, N>& mask) {
    for (int i = 0; i < N; i++) {
      _allInds[i] = i;
      if (mask[i]) {
        _trueInds[_trueCount++] = i;
      } else {
        _falseInds[_falseCount++] = i;
      }
    }
  }

  Array<int> trueInds() const {
    return Array<int>(_trueCount, _trueInds.data());
  }

  Array<int> falseInds() const {
    return Array<int>(_falseCount, _falseInds.data());
  }

  Array<int> allInds() const {
    return Array<int>(N, _allInds.data());
  }
private:
  int _trueCount = 0;
  int _falseCount = 0;
  std::array<int, N> _trueInds, _falseInds, _allInds;
};


/*
 * Represents a quadratic form
 *
 * X'PX - 2X'Q + R,
 *
 * where P and R are symmetric matrices.
 *
 * This is useful for building least squares problems
 * incrementally.
 *
 * Usually Q would have one column and R would be a 1x1 matrix.
 * However, in order to allow for least-squares problems with multiple
 * right-hand sides, we allow for more columns.
 *
 *
 *
 * |AX - B|^2 = (X'A' - B')(AX - B)
 *
 *    = X'A'AX + X'Q + Q'X + B'B
 *
 *                  = [P = A'A, Q = -A'B, R = B'B]
 *
 *    = X'PX + X'Q + Q'X + R
 */
template <int lhsDims, int rhsDims, typename T = double>
class QuadForm {
 public:
  // Ideally, this constructor
  // should be private, but it needs to
  // public if I want to allocate an array of QuadForms.
  QuadForm() {
    setConstant(T(0.0));
  }

  typedef QuadForm<lhsDims, rhsDims, T> ThisType;
  static constexpr int pDims = calcSymmetricMatrixStorageSize(lhsDims);
  static constexpr int pNumel = lhsDims*lhsDims;

  static constexpr int qDims = lhsDims*rhsDims;
  static constexpr int qNumel = lhsDims*rhsDims;

  static constexpr int rDims = calcSymmetricMatrixStorageSize(rhsDims);
  static constexpr int rNumel = rhsDims*rhsDims;

  static constexpr bool isStraightLineFit = lhsDims == 2 && rhsDims == 1;

  static constexpr int outDims = qDims;

  /*
   * Make a QuadForm from
   *
   * |A*X - B|^2
   *
   * where A is a 1*xDims matrix and
   *       B is a 1*rhsDims matrix
   *
   * Can be used to fit a model to
   * observations in the least squares sense.
   */
  static ThisType fit(const T *A, const T *B) {
    ThisType dst;
    for (int i = 0; i < lhsDims; i++) {
      for (int j = 0; j <= i; j++) {
        dst.setP(i, j, A[i]*A[j]);
      }
      for (int j = 0; j < rhsDims; j++) {
        dst.setQ(i, j, A[i]*B[j]);
      }
    }
    for (int i = 0; i < rhsDims; i++) {
      for (int j = 0; j <= i; j++) {
        dst.setR(i, j, B[i]*B[j]);
      }
    }
    return dst;
  }

  // Create a quad form with a diagonal matrix,
  // for regularization purposes to make problems well-posed.
  static ThisType makeReg(T lambda) {
    ThisType result(T(0));
    for (int i = 0; i < lhsDims; i++) {
      result.setP(i, i, lambda);
    }
    return result;
  }

  /*
   * Create a constant squared form,
   * that is a squared form whose value
   * does not depend on the vector X.
   *
   * This constructor is useful in generic algorithms that
   * can also work with numbers that need to be initialized to
   * zero, that is
   *
   * SomeUnknownTypeThatCouldBeAQuadFormOrASimplePrimitiveSuchAsDouble x = 0;
   */
  QuadForm(T x) {
    setConstant(x);
  }

  void setConstant(T x) {
    for (int i = 0; i < pDims; i++) {
      _P[i] = T(0);
    }
    for (int i = 0; i < qDims; i++) {
      _Q[i] = T(0);
    }
    for (int i = 0; i < rDims; i++) {
      _R[0] = T(x);
    }
  }

  ThisType operator+(const ThisType &other) const {
    ThisType dst;
    add(pDims, _P, other._P, dst._P);
    add(qDims, _Q, other._Q, dst._Q);
    add(rDims, _R, other._R, dst._R);
    return dst;
  }

  ThisType operator-(const ThisType &other) const {
    ThisType dst;
    subtract(pDims, _P, other._P, dst._P);
    subtract(qDims, _Q, other._Q, dst._Q);
    subtract(rDims, _R, other._R, dst._R);
    return dst;
  }


  void operator+=(const ThisType &other) {
    add(pDims, _P, other._P, _P);
    add(qDims, _Q, other._Q, _Q);
    add(rDims, _R, other._R, _R);
  }

  ThisType scale(T factor) const {
    ThisType dst;
    sail::scale(pDims, factor, _P, dst._P);
    sail::scale(qDims, factor, _Q, dst._Q);
    sail::scale(rDims, factor, _R, dst._R);
    return dst;
  }

  MDArray<T, 2> minimize() const {
    T pData[pNumel];
    T qData[qNumel];
    MDArray<T, 2> P(lhsDims, lhsDims, pData);
    MDArray<T, 2> Q(lhsDims, rhsDims, qData);
    fillPArray(P);
    fillQArray(Q);
    MDArray<T, 2> dst(lhsDims, rhsDims);
    LUImpl::solveLinearSystemLU(P, Q, &dst);
    return dst;
  }

  Eigen::Matrix<T, lhsDims, lhsDims> eigenP() const {
    Eigen::Matrix<T, lhsDims, lhsDims> P;
    for (int i = 0; i < lhsDims; i++) {
      for (int j = 0; j < lhsDims; j++) {
        P(i, j) = _P[calcSymmetricMatrixIndex(i, j)];
      }
    }
    return P;
  }

  Eigen::Matrix<T, lhsDims, rhsDims> eigenQ() const {
    Eigen::Matrix<T, lhsDims, rhsDims> Q;
    for (int i = 0; i < lhsDims; i++) {
      for (int j = 0; j < rhsDims; j++) {
        Q(i, j) = _Q[calcQIndex(i, j)];
      }
    }
    return Q;
  }

  Eigen::Matrix<T, rhsDims, rhsDims> eigenR() const {
    Eigen::Matrix<T, rhsDims, rhsDims> R;
    for (int i = 0; i < rhsDims; i++) {
      for (int j = 0; j < rhsDims; j++) {
        R(i, j) = _R[calcSymmetricMatrixIndex(i, j)];
      }
    }
    return R;
  }

  Eigen::Matrix<double, lhsDims, rhsDims> minimizeUsingEigen() const {
    Eigen::Matrix<double, lhsDims, lhsDims> P = eigenP();
    Eigen::Matrix<double, lhsDims, rhsDims> Q = eigenQ();
    return P.ldlt().solve(Q);
  }

  T evaluateEigenCost(
      const Eigen::Matrix<double, lhsDims, rhsDims>& X) const {
    auto Q = eigenQ();
    return (X.transpose()*eigenP()*X
        - (X.transpose()*eigenQ() + eigenQ().transpose()*X)
        + eigenR()).trace();
  }

  T evaluateOptimalEigenCost() const {
      return evaluateEigenCost(minimizeUsingEigen());
  }

  bool minimize2x1(T *out2) const {
    static_assert(isStraightLineFit, "Only for 2x1 quad forms");
    T ata[4] = {_P[0], _P[1], _P[1], _P[2]};
    T ataInv[4];
    bool ret = invert2x2(ata, ataInv);
    out2[0] = ataInv[0]*_Q[0] + ataInv[1]*_Q[1];
    out2[1] = ataInv[2]*_Q[0] + ataInv[3]*_Q[1];
    return ret;
  }

  T minimize1x1() const {
    static_assert(lhsDims == 1 && rhsDims == 1, "Only applicable to 1x1 quad forms");
    return _Q[0]/_P[0];
  }

  GenericLineKM<T> makeLine() const {
    T coefs[2];
    if (minimize2x1(coefs)) {
      return GenericLineKM<T>(coefs[0], coefs[1]);
    } else {
      return GenericLineKM<T>::undefined();
    }
  }

  T eval(const T *x) const {
    static_assert(rhsDims == 1 || rhsDims == 0, "Bad rhsDims value");
    T temp[lhsDims];
    for (int i = 0; i < lhsDims; i++) {
      T acc = (rhsDims == 0? T(0) : T(-2.0)*_Q[i]);
      for (int j = 0; j < lhsDims; j++) {
        acc += _P[calcSymmetricMatrixIndex(i, j)]*x[j];
      }
      temp[i] = acc;
    }
    T value = (rhsDims == 0? T(0) : T(_R[0]));
    for (int i = 0; i < lhsDims; i++) {
      value += x[i]*temp[i];
    }
    return value;
  }

  T evalOpt2x1() const {
    T coefs[2];
    minimize2x1(coefs);
    return eval(coefs);
  }

  static ThisType fitPolynomial(T x, T y) {
    static_assert(rhsDims == 1, "Only fitting to single values");
    T a[lhsDims];
    T prod = T(1);
    for (int i = 0; i < lhsDims; i++) {
      a[i] = prod;
      prod *= x;
    }
    return ThisType::fit(a, &y);
  }

  // Used to fit a straight line between a value x and a corresponding value y.
  static ThisType fitLine(T x, T y) {
    static_assert(isStraightLineFit, "Only to be used for line fitting");
    T a[2] = {x, T(1.0)};
    return ThisType::fit(a, &y);
  }

  // When this is a line fit, this is the number of equations.
  T lineFitCount() const {
    static_assert(isStraightLineFit, "Only applicable to straight line fits.");
    return _P[2];
  }

  T lineFitX() const {
    return _P[1]/lineFitCount();
  }

  T lineFitY() const {
    return _Q[1]/lineFitCount();
  }

  T pElement(int i, int j) const {
    assert(0 <= i && i < lhsDims);
    assert(0 <= j && j < lhsDims);
    return _P[calcSymmetricMatrixIndex(i, j)];
  }

  T qElement(int i, int j) const {
    assert(0 <= i && i < lhsDims);
    assert(0 <= j && j < rhsDims);
    return _Q[calcQIndex(i, j)];
  }

  T rElement(int i, int j) const {
    assert(0 <= i && i < rhsDims);
    assert(0 <= j && j < rhsDims);
    return _R[calcSymmetricMatrixIndex(i, j)];
  }

  void setP(int i, int j, T value) {
    assert(0 <= i && i < lhsDims);
    assert(0 <= j && j < lhsDims);
    _P[calcSymmetricMatrixIndex(i, j)] = value;
  }

  void setQ(int i, int j, T value) {
    assert(0 <= i && i < lhsDims);
    assert(0 <= j && j < rhsDims);
    _Q[calcQIndex(i, j)] = value;
  }

  void setR(int i, int j, T value) {
    assert(0 <= i && i < rhsDims);
    assert(0 <= j && j < rhsDims);
    _R[calcSymmetricMatrixIndex(i, j)] = value;
  }

  template <int n>
  QuadForm<n, rhsDims, T> zeroPaddedSlice(
      int offset) const {
    QuadForm<n, rhsDims, T> dst(T(0.0));
    int l = std::max(0, -offset);
    int u = std::min(n, lhsDims - offset);
    for (int i = l; i < u; i++) {
      int srcI = i + offset;
      for (int j = i; j < u; j++) {
        int srcJ = j + offset;
        dst.setP(i, j, pElement(srcI, srcJ));
      }
      for (int j = 0; j < rhsDims; j++) {
        dst.setQ(i, j, qElement(srcI, j));
      }
    }
    for (int i = 0; i < rhsDims; i++) {
      for (int j = i; j < rhsDims; j++) {
        dst.setR(i, j, rElement(i, j));
      }
    }
    return dst;
  }



/*
 * Given that we are trying to minimize |Ax*X + Ay*Y - B|^2
 *  w.r.t. to X, and Y is implicit.
 *
 *  First we introduce R = (Ay'*Ay)^{-1}*Ay'
 *
 *                     then P = -R*Ax
 *                          Q = R*B
 *
 *  The simplified quadratic form, with Y being implicit,
 *  can now be written as:
 *
 *  Second order term:
 *  X'...X: X'(Ax'Ax + Ax'*Ay*P + P'*Ay'*Ax + P'*Ay'*Ay*P)*X
 *
 *  First order terms:
 *  X'... : X'(Ax'*Ay*Q - Ax'*B + P'*Ay'*Ay*Q - P'*Ay'*B)
 *
 *  ....X : (Q'*Ay'*Ax + Q'*Ay'*Ay*P - B'*Ax - B'*Ay*P)X
 *
 *  Constant term:
 *    ... : B'*B - Q'*Ay'*B - B'*Ay*Q
 *
 *  OBSERVATIONS:
 *  Q never appears alone. It is always together with Ay: Ay*Q
 *  P never appears alone. It is always together with Ay: Ay*P
 *
 */
  ThisType eliminate(const std::array<bool, lhsDims>& toKeepMask) const {
    typedef Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> Mat;
    IndexSplit<lhsDims> split(toKeepMask);

    int rhsIndsData[rhsDims];
    for (int i = 0; i < rhsDims; i++) {
      rhsIndsData[i] = i;
    }
    auto rhsInds = Array<int>(rhsDims, rhsIndsData);

    Mat AtA = eigenP();
    Mat AtB = -eigenQ();
    Mat BtB = eigenR();

    Mat Axx = subMatrixByInds(AtA, split.trueInds(), split.trueInds());
    Mat Axy = subMatrixByInds(AtA, split.trueInds(), split.falseInds());
    Mat Ayx = subMatrixByInds(AtA, split.falseInds(), split.trueInds());
    Mat Ayy = subMatrixByInds(AtA, split.falseInds(), split.falseInds());
    Mat AytB = subMatrixByInds(AtB, split.falseInds(), rhsInds);
    Mat AxtB = subMatrixByInds(AtB, split.trueInds(), rhsInds);

    Eigen::LDLT<Mat> AyyDecomp(Ayy);
    Mat Q = AyyDecomp.solve(AytB);
    Mat P = AyyDecomp.solve(-Ayx);

    Mat QtAytB = Q.transpose()*AytB;
    Mat newAtA = Axx + Axy*P + P.transpose()*(Ayx + Ayy*P);
    Mat newQ = Axy*Q - AxtB + P.transpose()*(Ayy*Q - AytB);
    Mat newBtB = BtB + -QtAytB - QtAytB.transpose() + Q.transpose()*Ayy*Q;

    // Populate the upper-left parts of the matrices
    ThisType dst;
    for (int i = 0; i < newAtA.rows(); i++) {
      for (int j = i; j < newAtA.cols(); j++) {
        dst.setP(i, j, newAtA(i, j));
      }
      for (int j = 0; j < rhsDims; j++) {
        dst.setQ(i, j, newQ(i, j));
      }
    }
    for (int i = 0; i < newBtB.rows(); i++) {
      for (int j = i; j < newBtB.cols(); j++) {
        dst.setR(i, j, newBtB(i, j));
      }
    }
    return dst;
  }

  void toOstream(std::ostream* dst) const {
    *dst << "\nP:\n" << eigenP()
         << "\nQ:\n" << eigenQ()
         << "\nR:\n" << eigenR() << "\n\n";
  }
 private:
  void fillPArray(MDArray<T, 2> dst) const {
    assert(dst.rows() == lhsDims);
    assert(dst.cols() == lhsDims);
    for (int i = 0; i < lhsDims; i++) {
      for (int j = 0; j < lhsDims; j++) {
        dst(i, j) = _P[calcSymmetricMatrixIndex(i, j)];
      }
    }
  }

  void fillQArray(MDArray<T, 2> dst) const {
    assert(dst.rows() == lhsDims);
    assert(dst.cols() == rhsDims);
    for (int i = 0; i < lhsDims; i++) {
      for (int j = 0; j < rhsDims; j++) {
        dst(i, j) = _Q[calcQIndex(i, j)];
      }
    }
  }

  int calcQIndex(int i, int j) const {
    return i + j*lhsDims;
  }

  T _P[pDims];
  T _Q[qDims];
  T _R[rDims];
};

typedef QuadForm<2, 1, double> LineFitQF;

template <int lhsDims, int rhsDims, typename T>
QuadForm<lhsDims, rhsDims, T> operator* (T s, const QuadForm<lhsDims, rhsDims, T> &x) {
  return x.scale(s);
}


}




#endif /* QUADFUN_H_ */
