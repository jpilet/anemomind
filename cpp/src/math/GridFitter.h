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

	int getNLParamCount();

	virtual ~GridFit() {}
private:
	arma::sp_mat _P;
	ADFunction *_data;
	Array<arma::sp_mat> _regMatrices;
	Array<Arrayb> _splits;
	double _weight;

	Arrayd _regWeights;
};

typedef std::shared_ptr<GridFit> GridFitPtr;

class GridFitter
{
public:
	GridFitter();
	virtual ~GridFitter();

	void add(GridFitPtr gf);
	void add(GridFit *gf);

	void solve(Arrayd &X);

	int getNLParamCount();
private:
	std::vector<GridFitPtr> _terms;
};

Arrayb makeRandomSplit(int size);
Array<Arrayb> makeRandomSplits(int numSplits, int size);

arma::mat makeLsqDataToParamMat(arma::sp_mat P, Array<arma::sp_mat> A, Arrayd weights);


} /* namespace sail */

#endif /* GRIDFITTER_H_ */
