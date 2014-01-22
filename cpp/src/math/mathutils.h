/*
 * mathutils.h
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#ifndef MATHUTILS_H_
#define MATHUTILS_H_

#include <armadillo>
#include "../common/MDArray.h"

namespace arma
{

typedef arma::Mat<double>::fixed<2, 3> mat23;
typedef arma::Mat<double>::fixed<3, 2> mat32;

} /* namespace arma */

namespace sail
{

template <int dims>
MDArray2d toRows(Array<arma::vec::fixed<dims> > vecs)
{
	int count = vecs.size();
	MDArray2d dst(count, dims);
	for (int i = 0; i < count; i++)
	{
		double *v = vecs[i].memptr();
		for (int j = 0; j < dims; j++)
		{
			dst(i, j) = v[j];
		}
	}
	return dst;
}

#define MAKEDENSE(X) ((X) + arma::zeros((X).n_rows, (X.n_cols)))

// Makes a sparse matrix to select elements from a vector.
arma::sp_mat makeSpSel(Arrayb sel);

}

#endif /* MATHUTILS_H_ */
