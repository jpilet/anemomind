/*
 * GridFitter.h
 *
 *  Created on: 20 janv. 2014
 *      Author: jonas
 */

#ifndef GRIDFITTER_H_
#define GRIDFITTER_H_

#include "../common/Array.h"
#include <memory>
#include <vector>
#include <armadillo>
#include <list>

namespace sail
{

// This class holds one nonlinearly parameterized datavector to which we want to fit a grid and tune
// regularization parameters.

class ADFunction;
class GridFit
{
public:
	GridFit();
	GridFit(arma::sp_mat P, ADFunction *data, Array<arma::sp_mat> regMatrices, Array<Arrayb> splits,
			double weight = 1);


	ADFunction &getData() {return *_data;}

	int getNLParamCount() const;

	// Selects and slices a subset of the rows of _P
	arma::sp_mat makePsel(Arrayb sel);


	// Returns a matrix that, when multiplied by a data vector,
	// gives the least squares fit of the grid vertices to that data vector.
	// Optionally also takes a boolean array that select which elements of
	// the data vector to be used for the fit.
	arma::mat makeDataToParamMat(Arrayb sel = Arrayb());

	// Takes the above matrix and multiplies it with the various cost matrices inside
	// If D is the datavector and this function returns a matrix A,
	// then we want to minimize |A*D| w.r.t. D
	arma::mat makeDataToResidualsMat(Arrayb sel = Arrayb());

	virtual ~GridFit() {}
private:
	static arma::mat makeDataResidualMatSub(arma::sp_mat P, Array<arma::sp_mat> A, Arrayd weights);
	static arma::mat makeLsqDataToParamMatSub(arma::sp_mat P, Array<arma::sp_mat> A, Arrayd weights);

	arma::sp_mat _P;
	ADFunction *_data;
	Array<arma::sp_mat> _regMatrices;
	Array<Arrayb> _splits;

	Arrayd _regWeights;


	// How much THE WHOLE function is weighted
	double _weight;
};

class GridFitter
{
public:
	GridFitter();
	virtual ~GridFitter();

	// GridFitter will allocate a copy of gf on the heap,
	// acquire ownership of this copy and return a pointer to it.
	void add(std::shared_ptr<GridFit> gf);

	void solve(Arrayd &X);

	int getNLParamCount();
private:
	std::vector<std::shared_ptr<GridFit> > _terms;
};

Arrayb makeRandomSplit(int size);
Array<Arrayb> makeRandomSplits(int numSplits, int size);

} /* namespace sail */

#endif /* GRIDFITTER_H_ */
