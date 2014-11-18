#include "armaadolc.h"
#include <adolc/taping.h>
#include <adolc/drivers/drivers.h>
#include <adolc/sparse/sparsedrivers.h>
#include <server/common/MDArray.h>
#include <adolc/taping.h>

namespace arma {

using namespace sail;



arma::Mat<adouble> adolcInput(int count, double *src) {
  arma::Mat<adouble> Y(count, 1);
  for (int i = 0; i < count; i++) {
    Y(i, 0) <<= src[i];
  }
  return Y;
}

arma::Mat<adouble> adolcInputRowMajor(int rows, int cols, double *src) {
  arma::Mat<adouble> Y(rows, cols);
  int counter = 0;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      Y(i, j) <<= src[counter];
      counter++;
    }
  }
  return Y;
}


arma::Mat<adouble> adolcInput(const mat &X) {
  int rows = X.n_rows;
  int cols = X.n_cols;
  arma::Mat<adouble> Y(rows, cols);
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      Y(i, j) <<= X(i, j);
    }
  }
  return Y;
}


void adolcOutput(admat &src, mat &dst) {
  int rows = src.n_rows;
  int cols = src.n_cols;
  dst = mat(rows, cols);
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      src(i, j) >>= dst(i, j);
    }
  }
}

void adolcOutput(admat &src, double *dst) {
  int count = src.n_rows;
  assert(src.n_cols == 1);
  for (int i = 0; i < count; i++) {
    src(i, 0) >>= dst[i];
  }
}


void getRowPtrs(double *src, int m, int n,  double **dst) {
  for (int i = 0; i < m; i++) {
    dst[i] = src + i*n;
  }
}


mat getJacobianTranspose(short int tag, const mat &X) {
  unsigned long int counts[30];
  tapestats(tag, counts);
  int m = counts[1];
  int n = counts[0];
  assert(m > 0);
  assert(n > 0);

  arma::mat Jt(n, m);

  double **dst = new  double*[m];
  getRowPtrs(Jt.memptr(), m, n, dst);
  jacobian(tag, m, n, X.memptr(), dst);
  delete[] dst;

  return Jt;
}

mat getJacobian(short int tag, const mat &X) {
  return getJacobianTranspose(tag, X).t();
}


vec3 getValue(advec3 x) {
  double data[3] = {x(0).getValue(), x(1).getValue(), x(2).getValue()};
  return vec3(data);
}

}


namespace sail {

using namespace arma;

void dispAdolcInfo(unsigned short tag) {
  size_t counts[30];
  tapestats(tag, counts);
  std::cout << "counts[0] = " << counts[0] << "  number of independents, i.e. calls to = ," << std::endl;
  std::cout << "counts[1] = " << counts[1] << "  number of dependents, i.e. calls to = ," << std::endl;
  std::cout << "counts[2] = " << counts[2] << "  maximal number of live active variables," << std::endl;
  std::cout << "counts[3] = " << counts[3] << "  size of value stack (number of overwrites)," << std::endl;
  std::cout << "counts[4] = " << counts[4] << "  buffer size (a multiple of eight),2.2" << std::endl;
  std::cout << "counts[5] = " << counts[5] << "  the total number of operations recorded," << std::endl;

}

bool hasNaN(const arma::admat &X) {
  for (int i = 0; i < X.n_rows; i++) {
    for (int j = 0; j < X.n_cols; j++) {
      if (std::isnan(X(i, j).getValue())) {
        return true;
      }
    }
  }
  return false;
}

bool hasNaN(Array<adouble> X) {
  int count = X.size();
  for (int i = 0; i < count; i++) {
    if (std::isnan(X[i].getValue())) {
      return true;
    }
  }
  return false;
}




Array<adouble> adolcInput(Array<double> X) {
  int count = X.size();
  Array<adouble> Y(count);
  for (int i = 0; i < count; i++) {
    Y[i] <<= X[i];
  }
  return Y;
}


Array<adouble> adolcInput(int count, const double *src) {
  Array<adouble> dst(count);
  adolcInput(count, dst.ptr(), src);
  return dst;
}

Array<double> getGradient(short int tapeIndex, Array<double> X) {
  int dims = X.size();
  Array<double> grad(dims);
  gradient(tapeIndex, dims, X.getData(), grad.getData());
  return grad;
}

void outputGradient(short int tapeIndex, Array<double> X, double *grad) {
  int dims = X.size();
  gradient(tapeIndex, dims, X.getData(), grad);
}

void adolcOutput(Array<adouble> src, double *dst) {
  int count = src.size();
  for (int i = 0; i < count; i++) {
    src[i] >>= dst[i];
  }
}

void adolcOutput(Array<adouble> src, Array<double> &dst) {
  int count = src.size();
  dst.create(count);
  for (int i = 0; i < count; i++) {
    src[i] >>= dst[i];
  }
}

void outputJacobianColMajor(short int tag, double *X, double *J, int step) {
  unsigned long int counts[30];
  tapestats(tag, counts);
  int m = counts[1];
  int n = counts[0];
  assert(m > 0);
  assert(n > 0);
  if (step == -1) {
    step = m;
  }

  double *Jrow = new double[m*n];
  int count = m*n;
  for (int i = 0; i < count; i++) {
    Jrow[i] = 0.0;
  }
  double **dst = new  double*[m];

  getRowPtrs(Jrow, m, n, dst);
  assert(dst[0] == Jrow);

  jacobian(tag, m, n, X, dst);

  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      J[i + j*step] = Jrow[i*n + j];
      //assert(!std::isnan(J[i + j*m]));
    }
  }

  delete[] Jrow;
  delete[] dst;
}



void outputJacobianRowMajor(short int tag, double *X, double *Jrow) {
  unsigned long int counts[30];
  tapestats(tag, counts);
  int m = counts[1];
  int n = counts[0];
  assert(m > 0);
  assert(n > 0);


  int count = m*n;
  for (int i = 0; i < count; i++) {
    Jrow[i] = 0.0;
  }
  double **dst = new double*[m];

  getRowPtrs(Jrow, m, n, dst);
  assert(dst[0] == Jrow);

  jacobian(tag, m, n, X, dst);


  delete[] dst;
}


void outputJacobianColMajor(short int tag, Array<double> X, MDArray<double, 2> &J) {
  unsigned long int counts[30];
  tapestats(tag, counts);
  int m = counts[1];
  int n = counts[0];
  assert(m > 0);
  assert(n > 0);
  J.create(m, n);
  assert(X.size() == n);
  int step = m;
  outputJacobianColMajor(tag, X.getData(), J.getData(), step);
}

MDArray<double, 2> getJacobianArray(short int tag, Array<double> X) {
  MDArray<double, 2> J;
  outputJacobianColMajor(tag, X, J);
  return J;
}

void adolcOutput(int count, adouble *src, double *dst) {
  for (int i = 0; i < count; i++) {
    src[i] >>= dst[i];
  }
}

void adolcInput(int count, adouble *dst, const double *src) {
  for (int i = 0; i < count; i++) {
    dst[i] <<= src[i];
  }
}





Array<int> getNanEntries(Array<adouble> X) {
  std::vector<int> inds;
  int count = X.size();
  for (int i = 0; i < count; i++) {
    if (std::isnan(X[i].getValue())) {
      inds.push_back(i);
    }
  }
  return Array<int>::makeArrayCopy(inds);
}

arma::admat adMatMul(arma::admat A, arma::admat B) {
  int rows = A.n_rows;
  int cols = B.n_cols;
  int middle = A.n_cols;
  assert(middle == B.n_rows);

  int astep = A.n_rows;
  int bstep = 1;

  arma::admat result(rows, cols);
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      adouble sum = 0.0;
      adouble *a = A.memptr() + i;
      adouble *b = B.memptr() + j*B.n_rows;
      for (int k = 0; k < middle; k++) {
        sum += a[0]*b[0];
        a += astep;
        b += bstep;
      }
      result(i, j) = sum;
    }
  }
  return result;
}


}
