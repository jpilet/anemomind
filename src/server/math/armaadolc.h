/*
 * ADFunction.h
 *
 *  Created on: 20 janv. 2014
 *      Author: jonas
 *
 *  Type traits and helper functions to use the
 *  ADOL-C adouble type together with the Armadillo data types.
 */
#ifndef ARMAADOLC_H_
#define ARMAADOLC_H_

#include <adolc/adouble.h>
#include <armadillo>
#include "../common/MDArray.h"

namespace arma {


template<>
struct is_real<adouble> {
  static const bool value = true;
};

template <>
struct is_supported_elem_type<adouble> {
  static const bool value = true;
};

template<> struct is_non_integral<adouble> {
  static const bool value = true;
};

typedef Col<adouble> advec;
typedef advec::fixed<2> advec2;
typedef advec::fixed<3> advec3;
typedef advec::fixed<4> advec4;
typedef advec::fixed<5> advec5;
typedef advec::fixed<6> advec6;
typedef advec::fixed<7> advec7;
typedef advec::fixed<8> advec8;
typedef advec::fixed<9> advec9;

template<> struct is_promotable<adouble, double> : public is_promotable_ok {
  typedef adouble result;
};
template<> struct is_promotable<double, adouble> : public is_promotable_ok {
  typedef adouble result;
};
//template<> struct arma_real_or_cx_only< adouble >               { typedef adouble               result; };

typedef Mat<adouble> admat_;
//typedef sail::SafeMat<adouble> admat;
typedef admat_ admat;

// restrictors.hpp
template<> struct arma_scalar_only<adouble> {
  typedef adouble result;
};
template<> struct arma_signed_only<adouble> {
  typedef adouble result;
};
template<> struct arma_real_only<adouble> {
  typedef adouble result;
};
template<> struct arma_real_or_cx_only< adouble >               {
  typedef adouble               result;
};

typedef admat_::fixed<2, 2> admat22;
typedef admat_::fixed<2, 3> admat23;
typedef admat_::fixed<3, 2> admat32;
typedef admat_::fixed<3, 3> admat33;
typedef admat_::fixed<3, 4> admat34;
typedef admat_::fixed<4, 3> admat43;
typedef admat_::fixed<4, 4> admat44;




arma::Mat<adouble> adolcInput(int count, double *src);
arma::Mat<adouble> adolcInputRowMajor(int rows, int cols, double *src);
arma::Mat<adouble> adolcInput(const mat &X);
void adolcOutput(admat &src, mat &dst);
void adolcOutput(admat &src, double *dst);



mat getJacobian(short int tag, const mat &X);
mat getJacobianTranspose(short int tag, const mat &X);
inline void adolcOutput(adouble &src, double &dst) {
  src >>= dst;
}



inline arma::mat toDoubleMat(const arma::mat &src) {
  return src;
}

template <typename S, typename D>
void adolcOutputArray(sail::Array<S> src, sail::Array<D> &dst) {
  //return src.mapElements<D>([=] (S x) {return adolcOutput(x);});
  int count = src.size();
  dst = sail::Array<D>(count);
  for (int i = 0; i < count; i++) {
    adolcOutput(src[i], dst[i]);
  }
}


}

namespace sail {


typedef Array<adouble> Arrayad;
typedef MDArray<adouble, 2> MDArray2ad;

Array<double> getGradient(short int tapeIndex, Array<double> X);
void outputGradient(short int tapeIndex, Array<double> X, double *grad);
Array<adouble> adolcInput(Array<double> X);
Array<adouble> adolcInput(int count, double *x);
void adolcOutput(Array<adouble> src, double *dst);
void adolcOutput(Array<adouble> src, Array<double> &dst);
void outputJacobianRowMajor(short int tag, double *X, double *Jrow);
void outputJacobianColMajor(short int tag, double *X, double *J, int step = -1);
void outputJacobianColMajor(short int tag, Array<double> X, MDArray<double, 2> &J);
MDArray<double, 2> getJacobianArray(short int tag, Array<double> X);
void adolcOutput(int count, adouble *src, double *dst);
void adolcInput(int count, adouble *dst, double *src);


void dispAdolcInfo(unsigned short tag);
bool hasNaN(const arma::admat &X);
bool hasNaN(Array<adouble> X);



Array<int> getNanEntries(Array<double> X);
Array<int> getNanEntries(Array<adouble> X);

inline double getDouble(double x) {
  return x;
}
inline double getDouble(adouble x) {
  return x.getValue();
}

template <typename T>
MDArray<T, 2> toMDArray(arma::Mat<T> &x) {
  return MDArray<T, 2>(x.n_rows, x.n_cols, x.memptr());
}

arma::admat adMatMul(arma::admat A, arma::admat B);

//
}

namespace std {
inline bool isnan(adouble x) {
  return std::isnan(x.getValue());
}
}

#endif /* ARMAADOLC_H_ */
