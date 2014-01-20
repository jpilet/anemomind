/*
 * GridFitter.cpp
 *
 *  Created on: 20 janv. 2014
 *      Author: jonas
 */

#include "GridFitter.h"
#include "../common/Uniform.h"
#include "ADFunction.h"
#include "mathutils.h"
#include "../common/math.h"

namespace sail
{


GridFit::GridFit() : _weight(0.0), _data(nullptr)
{
}

GridFit::GridFit(arma::sp_mat P, ADFunction *data, Array<arma::sp_mat> regMatrices, Array<Arrayb> splits,
		double weight) : _P(P), _data(data), _regMatrices(regMatrices), _splits(splits), _weight(weight)
{
	_regWeights.create(regMatrices.size());
	_regWeights.setTo(1.0);
	assert(_data->outDims() == P.n_rows);
}


int GridFit::getNLParamCount()
{
	assert(_data != nullptr);
	return _data->inDims();
}

GridFitter::GridFitter()
{
	// TODO Auto-generated constructor stub

}



GridFitter::~GridFitter()
{
	// TODO Auto-generated destructor stub
}

void GridFitter::add(GridFitPtr gf)
{
	_terms.push_back(gf);
	assert(gf->getNLParamCount() == getNLParamCount());
}

void GridFitter::add(GridFit *gf)
{
	add(GridFitPtr(gf));
}

void GridFitter::solve(Arrayd &X)
{
	assert(X.size() == getNLParamCount());


}

int GridFitter::getNLParamCount()
{
	if (_terms.empty())
	{
		return -1;
	}
	else
	{
		return _terms[0]->getNLParamCount();
	}
}

Arrayb makeRandomSplit(int count)
{
	Uniform rng(0.0, 1.0);
	Arrayb split(count);
	for (int i = 0; i < count; i++)
	{
		split[i] = rng.gen() > 0.5;
	}
	return split;
}

Array<Arrayb> makeRandomSplits(int numSplits, int size)
{
	Array<Arrayb> dst(numSplits);
	for (int i = 0; i < numSplits; i++)
	{
		dst[i] = makeRandomSplit(size);
	}
	return dst;
}

arma::mat makeLsqDataToParamMat(arma::sp_mat P, Array<arma::sp_mat> A, Arrayd weights)
{
	int count = A.size();
	assert(count == weights.size());
	arma::sp_mat PtP = P.t()*P;
	arma::mat K = MAKEDENSE(PtP);
	for (int i = 0; i < count; i++)
	{
		arma::sp_mat a = A[i];
		K += sqr(weights[i])*(a.t()*a);
	}
	arma::sp_mat spPt = P.t();
	arma::mat Pt = MAKEDENSE(spPt);
	arma::mat result = arma::solve(K, Pt);
	return result;
}

} /* namespace sail */
