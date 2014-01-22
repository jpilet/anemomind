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

namespace
{
	int countRows(Array<arma::sp_mat> A)
	{
		int counter = 0;
		for (int i = 0; i < A.size(); i++)
		{
			counter += A[i].n_rows;
		}
		return counter;
	}
}


arma::mat makeDataResidualMat(const arma::mat &F,
		const arma::sp_mat &P, Array<arma::sp_mat> A, Arrayd weights)
{
	assert(A.size() == weights.size());
	int indims = F.n_cols;
	arma::mat residuesData = P*F - arma::eye(indims, indims);

	int rowcount = residuesData.n_rows + countRows(A);
	arma::mat dst(rowcount, indims);
	dst.fill(-1);
	int offset = residuesData.n_rows;
	dst.rows(0, offset-1) = residuesData;
	for (int i = 0; i < A.size(); i++)
	{
		int next = offset + A[i].n_rows;
		dst.rows(offset, next-1) = weights[i]*A[i]*F;

		offset = next;
	}
	assert(offset == rowcount);
	return dst;
}

arma::mat GridFit::makeDataResidualMatSub(arma::sp_mat P, Array<arma::sp_mat> A, Arrayd weights)
{
	return makeDataResidualMat(makeLsqDataToParamMatSub(P, A, weights), P, A, weights);
}

arma::mat GridFit::makeLsqDataToParamMatSub(arma::sp_mat P, Array<arma::sp_mat> A, Arrayd weights)
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


int GridFit::getNLParamCount() const
{
	assert(_data != nullptr);
	return _data->inDims();
}

arma::sp_mat GridFit::makePsel(Arrayb sel)
{
	if (sel.hasData())
	{
		return makeSpSel(sel)*_P;
	}
	else
	{
		return _P;
	}
}


arma::mat GridFit::makeDataToParamMat(Arrayb sel)
{
	arma::sp_mat Psel = makePsel(sel);
	return makeLsqDataToParamMatSub(Psel, _regMatrices, _regWeights);
}

arma::mat GridFit::makeDataToResidualsMat(Arrayb sel)
{
	arma::sp_mat Psel = makePsel(sel);
	return makeDataResidualMatSub(Psel, _regMatrices, _regWeights);
}

GridFitter::GridFitter()
{

}



GridFitter::~GridFitter()
{
}

GridFit &GridFitter::add(const GridFit &gf)
{
	//GridFit *gfptr = new GridFit(gf);
	//_terms.push_back(gfptr);
	//return gfptr;
	if (!_terms.empty())
	{
		assert(gf.getNLParamCount() == _terms.front().getNLParamCount());
	}
	_terms.push_back(gf);
	return _terms.front();
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
		return _terms.front().getNLParamCount();
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



} /* namespace sail */
