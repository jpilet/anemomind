/*
 *  Created on: 2014-03-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Armadillo interface to the code in LUImpl.h
 *
 *  Internally uses the code in the LUImpl.h file. The advantage
 *  of wrapping that code is a reduction in compilation time since
 *  we avoid instatiating the templates every time LU decomposition is used.
 */

#ifndef LU_H_
#define LU_H_

#include <server/math/armaadolc.h>

namespace sail {

// Easy-to-use functions to work with Armadillo
// These functions return
arma::mat solveLU(arma::mat A, arma::mat B);
arma::admat solveLU(arma::admat A, arma::admat B);

// Wrapper for the solveLinearSystemLU template function
// These functions are useful when we already have allocated memory
// to store the solution to the linear system.
void solveLUArrayOut(MDArray2d A, MDArray2d b, MDArray2d *xOut);
void solveLUArrayOut(MDArray2ad A, MDArray2ad b, MDArray2ad *xOut);

}

#endif /* LU_H_ */
