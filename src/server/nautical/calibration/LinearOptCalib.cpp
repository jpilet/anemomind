/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "LinearOptCalib.h"
#include <server/math/nonlinear/DataFit.h>
#include <server/common/string.h>
#include <server/common/ArrayBuilder.h>

namespace sail {
namespace LinearOptCalib {

using namespace Eigen;

template <typename T>
bool hasEvenRows(const T &x) {
  return isEven(x.rows());
}

MatrixXd orthonormalBasis(const MatrixXd &x) {
  return MatrixXd(
      HouseholderQR<MatrixXd>(x)
        .householderQ()*MatrixXd::Identity(
            x.rows(), x.cols()));
}

SparseVector::SparseVector(int dim, Array<Entry> entries) : _dim(dim), _entries(entries) {
  assert(std::is_sorted(entries.begin(), entries.end()));
}

SparseVector SparseVector::zeros(int dim) {
  return SparseVector(dim, Array<Entry>());
}

SparseVector operator*(double factor, const SparseVector &x) {
  if (factor == 0.0) {
    return SparseVector::zeros(x.dim());
  }
  return SparseVector(x.dim(), x.entries().map<SparseVector::Entry>(
  [=](const SparseVector::Entry &e) {
    return SparseVector::Entry{e.index, factor*e.value};
  }));
}


int commonIndex(const EntryPair &p) {
  return std::max(p.first.index, p.second.index);
}

SparseVector scaledAdd(
    double aFactor, const SparseVector &a,
    double bFactor, const SparseVector &b) {
  auto pairs = listPairs(a, b);
  auto entries = pairs.map<SparseVector::Entry>([&](const EntryPair &p) {
    return SparseVector::Entry(
        commonIndex(p),
        aFactor*p.first.valueOr0() + bFactor*p.second.valueOr0());
  });
  return SparseVector(a.dim(), entries);
}

SparseVector::Entry getEntry(const SparseVector &x, int i) {
  return (i < x.entries().size()? x.entries()[i] : SparseVector::Entry());
}

Array<EntryPair>
  listPairs(const SparseVector &a, const SparseVector &b) {
  int ai = 0;
  int bi = 0;
  typedef SparseVector::Entry E;
  ArrayBuilder<EntryPair> pairs;
  while (true) {
    auto ae = getEntry(a, ai);
    auto be = getEntry(b, bi);
    if (!ae.valid()) {
      if (!be.valid()) {
        return pairs.get();
      }
      pairs.add(EntryPair(E(), be));
      bi++;
    } else if (!be.valid()) {
      pairs.add(EntryPair(ae, E()));
      ai++;
    } else {
      if (ae.index == be.index) {
        pairs.add(EntryPair(ae, be));
        ai++;
        bi++;
      } else if (ae.index < be.index) {
        pairs.add(EntryPair(ae, E()));
        ai++;
      } else {
        pairs.add(EntryPair(E(), be));
        bi++;
      }
    }
  }
}


SparseVector operator+(const SparseVector &a, const SparseVector &b) {
  return scaledAdd(1.0, a, 1.0, b);
}

SparseVector operator-(const SparseVector &a, const SparseVector &b) {
  return scaledAdd(1.0, a, -1.0, b);
}

SparseVector operator-(const SparseVector &a) {
  return (-1.0)*a;
}

double dot(const SparseVector &a, const SparseVector &b) {
  auto pairs = listPairs(a, b);
  std::function<double(double,EntryPair)> pairProd = [&](double v, const EntryPair &e) {
      return v + e.first.valueOr0()*e.second.valueOr0();
    };
  auto result = pairs.reduce<double>(0.0, pairProd);
  return result;
}

double squaredNorm(const SparseVector &x) {
  double s = 0.0;
  for (auto e: x.entries()) {
    s += sqr(e.value);
  }
  return s;
}

double norm(const SparseVector &x) {
  return sqrt(squaredNorm(x));
}

SparseVector normalize(const SparseVector &x) {
  return (1.0/norm(x))*x;
}

SparseVector projectOnNormalized(const SparseVector &a, const SparseVector &bHat) {
  return dot(a, bHat)*bHat;
}

SparseVector project(const SparseVector &a, const SparseVector &b) {
  return projectOnNormalized(a, normalize(b));
}

std::ostream &operator<<(std::ostream &s, const SparseVector &x) {
  s << "SparseVector:\n";
  for (auto e: x.entries()) {
    s << "  x[" << e.index << "] = " << e.value << "\n";
  }
  return s;
}

std::ostream &operator<<(std::ostream &s, const SparseVector::Entry &e) {
  s << "Entry(index=" << e.index << ", " << e.value << ")";
  return s;
}

std::ostream &operator<<(std::ostream &s, const EntryPair &p) {
  s << "Pair(" << p.first << ", " << p.second << ")";
  return s;
}


int countFlowEqs(Array<Spani> spans) {
  return spans.reduce<int>(0, [](int x, Spani y) {return x + y.width();});
}

MatrixXd makeParameterizedApparentFlowMatrix(const MatrixXd &A, Array<Spani> spans) {
  int cols = A.cols();
  MatrixXd result(2*countFlowEqs(spans), cols);
  int from = 0;
  for (auto span: spans) {
    auto span2 = 2*span;
    int to = from + span2.width();
    result.block(from, 0, to-from, cols)
        = A.block(span2.minv(), 0, span2.width(), cols);
    from = to;
  }
  assert(from == result.rows());
  return result;
}

Array<Spani> makeOverlappingSpans(int dataSize, int spanSize, double relativeStep) {
  LineKM spanStart(0, 1, 0, relativeStep*spanSize);
  auto lastSpanIndex = spanStart.solveWithEquality(dataSize - spanSize);
  int spanCount = int(floor(lastSpanIndex)) + 1;
  return Spani(0, spanCount).map<Spani>([&](int spanIndex) {
    int from = int(round(spanStart(spanIndex)));
    return Spani(from, from + spanSize);
  });
}

void addSpanWithConstantFlow(Spani srcSpan, Spani dstSpan, int spanIndex, const VectorXd &B,
    std::vector<Triplet<double> > *dst) {
  assert(isEven(srcSpan.width()));
  assert(srcSpan.width() == dstSpan.width());
  int colOffset = 1 + 2*spanIndex;
  for (auto i: srcSpan.indices()) {
    int dstRow = dstSpan[i];
    dst->push_back(Eigen::Triplet<double>(dstRow, 0, B(srcSpan[i])));
    dst->push_back(Eigen::Triplet<double>(dstRow, colOffset + (isEven(i)? 0 : 1), 1.0));
  }
}

int calcIdealCols(int spanCount) {
  return  1 + 2*spanCount;
}

/*
Eigen::SparseMatrix<double> assembleSpansWithConstantFlow(
    Eigen::VectorXd B, Array<Spani> spans) {
  std::sort(spans.begin(), spans.end());
  int n = countFlowEqs(spans);
  int idealCols = calcIdealCols(spans.size());
  int rows = 2*n;
  assert(idealCols <= rows);
  SparseMatrix<double> dst(rows, idealCols);
  std::vector<Triplet<double> > triplets;
  int from = 0;
  for (int i = 0; i < spans.size(); i++) {
    auto span2 = 2*spans[i];
    auto to = from + span2.width();
    addSpanWithConstantFlow(span2, Spani(from, to), i, B, &triplets);
    from = to;
  }
  assert(from == dst.rows());
  dst.setFromTriplets(triplets.begin(), triplets.end());
  return dst;
}


  SparseMatrix<double> orthonormalBasis(Eigen::SparseMatrix<double> X, int idealCols) {
  SparseQR<SparseMatrix<double>, Eigen::COLAMDOrdering<int> > qr(X);
  SparseMatrix<double> q; //(X.rows(), X.cols());
  SparseMatrix<double> id(X.rows(), idealCols);
  id.setIdentity();
  q = (qr.matrixQ()*id);
  return q;
}

Eigen::SparseMatrix<double> makeRhs(const VectorXd &B, Array<Spani> spans) {
  auto assembled = assembleSpansWithConstantFlow(B, spans);
  return orthonormalBasis(assembled);
}*/


/*
 * Unfortunately, the SparseQR code of Eigen
 * doesn't seem to let us omit the nullspace,
 * which is going to be **lots** of nonzero elements.
 * So let's just do our own Gram-Schmidt to get an
 * orthonormal basis. Please reorder the sparse vectors
 * so that those with the fewest elements come first which
 * will give us the sparsest output.
 *
 * https://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process#Numerical_stability
 */
Array<SparseVector> gramSchmidt(
    Array<SparseVector> vectors) {
  int n = vectors.size();
  Array<SparseVector > result(n);
  for (int i = 0; i < n; i++) {
    SparseVector vk = vectors[i];
    SparseVector uk = vk;
    for (int j = 0; j < i; j++) {
      auto proj = projectOnNormalized(uk, result[j]);
      uk = uk - proj;
    }
    result[i] = normalize(uk);
  }
  return result;
}

//SparseMatrix<double> makeRhs();

class SparseVec {
 public:
  struct Element {
    int index;
    double value;
  };

  SparseVec(int count, std::function<Element(int)> elemFun) : _elemFun(elemFun), _count(count) {}

 private:
  int _count;
  std::function<Element(int)> _elemFun;
};

class ByWidth {
 public:
  bool operator() (const Spani &a, const Spani &b) const {
    return a.width() < b.width();
  }
};



Results optimize(
    const MatrixXd &A, const VectorXd &B,
    Array<Spani> spans0,
    const Settings &settings) {

  // Sort, so that the orthonormal basis becomes as sparse as possible.
  Array<Spani> spans = spans0.dup();
  std::sort(spans.begin(), spans.end(), ByWidth());

  assert(hasEvenRows(A));
  assert(hasEvenRows(B));
  assert(A.rows() == B.rows());

}


}
}
