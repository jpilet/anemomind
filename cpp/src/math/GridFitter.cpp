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
#include "mathutils.h"
#include "armaadolc.h"
#include "LevMar.h"
#include "LevmarSettings.h"
#include "pareto.h"
#include "../common/common.h"
#include "../common/StepMinimizer.h"
#include "../common/text.h"

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


arma::mat GridFit::makeNormalMat(arma::sp_mat P, Array<arma::sp_mat> A, Arrayd weights)
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
	return K;
}

arma::mat GridFit::makeLsqDataToParamMatSub(arma::sp_mat P, Array<arma::sp_mat> A, Arrayd weights)
{
	arma::mat K = makeNormalMat(P, A, weights);
	arma::sp_mat spPt = P.t();
	arma::mat Pt = MAKEDENSE(spPt);
	arma::mat result = arma::solve(K, Pt);
	return result;
}



GridFit::GridFit() : _weight(0.0), _data(nullptr)
{
}

GridFit::GridFit(arma::sp_mat P, ADFunction *data, Array<arma::sp_mat> regMatrices, Array<Arrayb> splits,
		Arrayd regWeights,
		Array<std::string> regWeightLabels,
		double weight) : _P(P), _data(data), _regMatrices(regMatrices), _splits(splits),
		_regWeights(regWeights),
		_labels(regWeightLabels),
		_weight(weight)
{
	assert(splits.hasData());
	assert(_labels.empty() || _labels.size() == _regWeights.size());
	assert(_regWeights.size() == _regMatrices.size());
	assert(_data->outDims() == P.n_rows);
	assert(_data->outDims() == splits[0].size());
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
	return _weight*makeDataResidualMatSub(Psel, _regMatrices, _regWeights);
}

void GridFit::setExpRegWeight(int index, double logX)
{
	setRegWeight(index, exp(logX));
}

std::string GridFit::getRegLabel(int index)
{
	return (_labels.empty()? stringFormat("regw(%d/%d)", index+1, _regWeights.size()) : _labels[index]);
}

arma::mat GridFit::makeCrossValidationFitnessMat()
{
	// true  <=> test
	// false <=> train

	int testCount = 0;
	int splitCount = _splits.size();
	for (int i = 0; i < splitCount; i++)
	{
		testCount += countTrue(_splits[i]);
	}

	arma::mat M(testCount, _data->outDims());

	int offset = 0;
	for (int i = 0; i < splitCount; i++)
	{
		Arrayb test = _splits[i];
		Arrayb train = neg(test);
		arma::sp_mat selTrain = makeSpSel(train);
		arma::sp_mat Ptrain = selTrain*_P;

		arma::mat fit = makeLsqDataToParamMatSub(Ptrain, _regMatrices, _regWeights);

		arma::sp_mat selTest = makeSpSel(test);
		arma::sp_mat Ptest = selTest*_P;
		int ptrows = Ptest.n_rows;
		int next = offset + ptrows;
		arma::mat FS = fit*selTrain;

		M.rows(offset, next-1) = selTest*(_P*FS - arma::eye(_P.n_rows, _P.n_rows));
		offset = next;
	}
	assert(offset == testCount);

	return M;
}

arma::mat GridFit::fitGridParamsForDataVectorAndWeights(arma::mat D, Arrayd weights, arma::sp_mat P, Array<arma::sp_mat> A)
{
	arma::mat K = arma::solve(GridFit::makeNormalMat(P, A, weights), P.t()*D);
	return K;
}

double GridFit::evalObjfForDataVector(arma::mat D)
{
	assert(D.n_rows == _data->outDims());
	assert(D.n_cols == 1);
	arma::mat params = fitGridParamsForDataVectorAndWeights(D, _regWeights, _P, _regMatrices);

	double cost = SQNORM(_P*params - D);
	int count = _regMatrices.size();
	for (int i = 0; i < count; i++)
	{
		arma::mat R = _regWeights[i]*_regMatrices[i]*params;
		cost += SQNORM(R);
	}

	double weightedCost = sqr(_weight)*cost;

	//assert(std::abs(weightedCost - SQNORM(makeDataToResidualsMat()*D)) < 1.0e-6); PFLMESSAGE("This function seems correct");



	return weightedCost;
}

double GridFit::evalCrossValidationFitness(arma::mat D)
{
	return SQNORM(makeCrossValidationFitnessMat()*D); // We may want to do something more efficient here, in future.
}

GridFitter::GridFitter()
{

}



GridFitter::~GridFitter()
{
}

void GridFitter::add(std::shared_ptr<GridFit> gf)
{
	if (!_terms.empty())
	{
		assert(gf->getNLParamCount() == getNLParamCount());
	}
	_terms.push_back(gf);
}




// Evaluates the objective function given the
// nonlinear parameter vector. Regularization weights
// are assumed to remain constant throughout the lifetime of
// this object.
class GridFitPlayer1 : public ADFunction
{
public:
	GridFitPlayer1(ParetoFrontier &frontier, std::vector<std::shared_ptr<GridFit> > &fits);

	int inDims() {return _inDims;}
	int outDims() {return _outDims;}
	void evalAD(adouble *Xin, adouble *Fout);

	bool acceptor(double *Xin, double objfVal);
private:
	ParetoFrontier &_frontier;

	std::vector<std::shared_ptr<GridFit> > &_fits; // The GridFit's

	// Matrices used to evaluate the main objective function (used by evalAD)
	Array<arma::mat> _Rmats;

	// Matrices used to compute cross validation.
	Array<arma::mat> _cvmats;

	int _outDims, _inDims, _maxDataLen;

	// Evaluates the cross validation score for every GridFit given the nonlinear parameter vector
	void evalCrossValidations(double *Xin, double *cvOut);
};

GridFitPlayer1::GridFitPlayer1(ParetoFrontier &frontier, std::vector<std::shared_ptr<GridFit> > &fits) :
		_frontier(frontier), _fits(fits)
{
	int count = fits.size();
	_Rmats.create(count);
	_cvmats.create(count);
	_inDims = fits[0]->getData().inDims();
	_outDims = 0;
	_maxDataLen = 0;
	for (int i = 0; i < count; i++)
	{
		GridFit &fit = *(fits[i]);
		assert(fit.getData().inDims() == _inDims);
		arma::mat R = fit.makeDataToResidualsMat();
		_Rmats[i] = R;
		_cvmats[i] = fit.makeCrossValidationFitnessMat();
		_outDims += R.n_rows;
		_maxDataLen = std::max(_maxDataLen, fit.getData().outDims());
	}
}

void GridFitPlayer1::evalAD(adouble *Xin, adouble *Fout)
{
	int count = _fits.size();
	int offset = 0;
	Array<adouble> temp(_maxDataLen);
	for (int i = 0; i < count; i++)
	{
		arma::mat &R = _Rmats[i];
		adouble *Fouti = Fout + offset;
		arma::admat dst(Fouti, R.n_rows, 1, false, true);
		ADFunction &data = _fits[i]->getData();
		arma::admat D(temp.getData(), data.outDims(), 1, false, true);
		data.evalAD(Xin, temp.getData());
		dst = R*D;
		offset += R.n_rows;
	}
	assert(offset == _outDims);
}

bool GridFitPlayer1::acceptor(double *Xin, double objfVal)
{
	int count = _fits.size();
	Arrayd costs(1 + count);
	evalCrossValidations(Xin, costs.ptr(1));
	return _frontier.insert(ParetoElement(costs));
}

void GridFitPlayer1::evalCrossValidations(double *Xin, double *cvOut)
{
	int count = _fits.size();
	arma::mat X(Xin, inDims(), 1, false, true);

	Arrayd temp(_maxDataLen);
	for (int i = 0; i < count; i++)
	{
		Function &fun = _fits[i]->getData();
		arma::mat D(temp.getData(), fun.outDims(), 1, false, true);
		fun.eval(Xin, temp.getData());
		cvOut[i] = SQNORM(_cvmats[i]*D);
	}
}







class GridFitOtherPlayers
{
public:
	GridFitOtherPlayers(ParetoFrontier &frontier, std::vector<std::shared_ptr<GridFit> > &fits, arma::mat X);
	void optimize(Array<Arrayd> stepSizes);
private:
	ParetoFrontier &_frontier;
	Array<arma::mat> _D;
	std::vector<std::shared_ptr<GridFit> > &_fits;

	void optimizeForGridFit(int index, Arrayd stepSizes);
	double evalObjf();
	ParetoElement makeParetoElementVector();
};

GridFitOtherPlayers::GridFitOtherPlayers(ParetoFrontier &frontier,
		std::vector<std::shared_ptr<GridFit> > &fits, arma::mat X) : _frontier(frontier), _fits(fits)
{
	int count = _fits.size();
	_D.create(count);
	for (int i = 0; i < count; i++)
	{
		Function &fun = _fits[i]->getData();
		_D[i] = arma::mat(fun.outDims(), 1);
		fun.eval(X.memptr(), _D[i].memptr());
	}
}

void GridFitOtherPlayers::optimize(Array<Arrayd> stepSizes)
{
	int count = _fits.size();
	for (int i = 0; i < count; i++)
	{
		optimizeForGridFit(i, stepSizes[i]);
	}
}

void GridFitOtherPlayers::optimizeForGridFit(int index, Arrayd stepSizes)
{
	GridFit &f = *(_fits[index]);
	for (int i = 0; i < f.getRegCount(); i++)
	{
		// It is best to do this search in the logarithmic domain,
		// because this way, the weight stays positive.
		double initReg = log(f.getRegWeight(i));


		double initStep = stepSizes[i];
		auto objf = [&] (double x) {f.setExpRegWeight(i, x);
			return f.evalCrossValidationFitness(_D[i]);};
		auto acceptor = [&] (double x, double val) {f.setExpRegWeight(i, x);
			return _frontier.insert(makeParetoElementVector());};
		StepMinimizerState initState(initReg, initStep, objf(initReg));
		StepMinimizer minimizer;
		minimizer.setAcceptor(acceptor);
		StepMinimizerState finalState = minimizer.takeStep(initState, objf);
		stepSizes[i] = finalState.getStep();
		f.setExpRegWeight(i, finalState.getX());
	}
}

double GridFitOtherPlayers::evalObjf()
{
	int count = _fits.size();
	double value = 0.0;
	for (int i = 0; i < count; i++)
	{
		value += _fits[i]->evalObjfForDataVector(_D[i]);
	}
	return value;
}

ParetoElement GridFitOtherPlayers::makeParetoElementVector()
{
	int count = _fits.size();
	Arrayd values(1 + count);
	values[0] = evalObjf();
	for (int i = 0; i < count; i++)
	{
		values[1 + i] = _fits[i]->evalCrossValidationFitness(_D[i]);
	}
	return ParetoElement(values);
}

Array<Arrayd> initStepSizes(Arrayi regCounts, double initStepSize)
{
	return regCounts.map<Arrayd>([=] (int n) {Arrayd dst(n); dst.setTo(initStepSize); return dst;});
}

void GridFitter::writeStatus(int i, arma::mat X, int fsize)
{ // Status output for this iteration
	if (i == 0)
	{
		cout << "################################################ GRIDFITTER BEGIN SOLVE" << endl;
	}


	if (i == -1)
	{
		cout << "######################## DONE SOLVING IT" << endl;
	}
	else
	{
		std::cout << "\n\n\n### GRIDFITTER ITERATION " << i+1 << std::endl;
	}

	std::cout << "   X = " << X.t() << endl;
	int gfCount = _terms.size();
	for (int i = 0; i < gfCount; i++)
	{
		std::shared_ptr<GridFit> gf = _terms[i];
		std::string l = gf->getLabel();
		std::cout << "   " << (l.empty()? stringFormat("GridFit(%d/%d)", i+1, gfCount) : l) << std::endl;
		int rCount = gf->getRegCount();
		for (int j = 0; j < rCount; j++)
		{
			std::cout << "     " << gf->getRegLabel(j) << " = " << gf->getRegWeight(j) << std::endl;
		}
	}
	std::cout << "   Frontier size: " << fsize << endl;
	if (i == -1)
	{
		cout << "########################################################################################" << endl;
	}
}

void GridFitter::solve(arma::mat &X)
{
	assert(X.size() == getNLParamCount());

	LevmarSettings settings;
	settings.verbosity = 0;

	const double initStepSize = 0.1;
	Array<Arrayd> stepSizes = initStepSizes(getRegCounts(), initStepSize);

	ParetoFrontier frontier;

	const int iters = 120;
	for (int i = 0; i < iters; i++)
	{
		writeStatus(i, X, frontier.size());

		// Part 1: Optimize Player 1 (the objective function)
		{
			GridFitPlayer1 objf(frontier, _terms);
			settings.acceptor = [&] (double *Xd, double val) {return objf.acceptor(Xd, val);};
			LevmarState lmState(X);
			lmState.step(settings, objf);
			X = lmState.getX();
		}

		// Part 2: Adjust the regularization weights of every grid fit.
		{
			GridFitOtherPlayers other(frontier, _terms, X);
			other.optimize(stepSizes);
		}
	}
	writeStatus(-1, X, frontier.size());
}

void GridFitter::solveFixedReg(arma::mat &X)
{
	LevmarState lmState(X);
	ParetoFrontier frontier;
	GridFitPlayer1 objf(frontier, _terms);
	LevmarSettings settings;
	settings.verbosity = 3;
	lmState.minimize(settings, objf);

	X = lmState.getX();
}

int GridFitter::getNLParamCount()
{
	if (_terms.empty())
	{
		return -1;
	}
	else
	{
		return _terms.front()->getNLParamCount();
	}
}

Arrayi GridFitter::getRegCounts()
{
	int count = _terms.size();
	Arrayi counts(count);
	for (int i = 0; i < count; i++)
	{
		counts[i] = _terms[i]->getRegCount();
	}
	return counts;
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
