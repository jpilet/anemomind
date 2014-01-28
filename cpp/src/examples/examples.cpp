/*
 * examples.cpp
 *
 *  Created on: 16 janv. 2014
 *      Author: jonas
 */

#include "examples.h"
#include "examples.h"
#include "../plot/gnuplot_i.hpp"
#include "../common/LineKM.h"
#include "../common/common.h"
#include <cmath>
#include <vector>
#include <iostream>
#include "../common/Array.h"
#include "../common/ArrayIO.h"
#include "../common/filesystem.h"
#include "Env.h"
#include "../common/Nav.h"
#include "../common/Duration.h"
#include "../common/ContinuousRange.h"
#include "../common/NavBBox.h"
#include "../math/LocalRace.h"
#include "../math/mathutils.h"
#include "../plot/extra.h"
#include "../math/pareto.h"
#include "../common/Uniform.h"
#include "../math/ADFunction.h"
#include "../math/GridFitter.h"

/*
 * examples.cpp
 *
 *  Created on: 16 janv. 2014
 *      Author: jonas
 */


namespace sail
{

void runTests()
{
	example006();
	example007();
	example009();
	example010();
	example011();
}

void ex001AddCircle(Gnuplot &plot)
{
	plot.set_style("lines");
	int count = 300;
	LineKM line(0, count-1, 0.0, 2.0*M_PI);

	std::vector<double> X(count), Y(count), Z(count);
	for (int i = 0; i < count; i++)
	{
		X[i] = cos(line(i));
		Y[i] = sin(line(i));
		Z[i] = (1.0/count)*i;
	}

//	plot.set_yrange(-2, 2);
//	plot.set_xrange(-2, 2);
//	plot.set_zrange(-1, 1);
	plot.plot_xyz(X, Y, Z);
}

void example001()
{
	Gnuplot plot;
	ex001AddCircle(plot);
	plot.set_xautoscale();
	plot.set_yautoscale();
	plot.set_zautoscale();
	plot.showonscreen();
	sleepForever();
}

void example002() // Checked with valgrind
{

	Arrayi test(3);
	for (int i = 0; i < test.size(); i++)
	{
		test[i] = i;
	}
	Arrayi test2 = test.dup();
	test = Arrayi();
	DOUT(test2);
}

void example003() // Checked with valgrind
{
	SysEnv env;
	Array<Nav> navs = loadNavsFromText(env.dataset.cat("allnavs.txt").str());
	assert(areSortedNavs(navs));

	//plotNavTimeVsIndex(navs);
}

void example004() //
{
	SysEnv env;
	Array<Nav> navs = loadNavsFromText(env.dataset.cat("allnavs.txt").str());
	dispNavTimeIntervals(navs);

	Array<Array<Nav> > splitNavs = splitNavsByDuration(navs, Duration::minutes(10).getDurationSeconds());
	DOUT(splitNavs.size());

	plotNavsEcefTrajectories(splitNavs);
}

void example005()
{
	SysEnv env;
	Array<Nav> navs = loadNavsFromText(env.dataset.cat("allnavs.txt").str(), false);
	Array<Array<Nav> > splitNavs = splitNavsByDuration(navs, Duration::minutes(10).getDurationSeconds());

	plotNavsEcefTrajectory(splitNavs.last());
	//plotNavsEcefTrajectory(splitNavs.last());
}

void example006()
{
	assert(ContinuousRange(Arrayd::args(1, 2), false).intersects(ContinuousRange(Arrayd::args(1.5, 3), false)));
	assert(!ContinuousRange(Arrayd::args(1, 2), false).intersects(ContinuousRange(Arrayd::args(3, 4), false)));
	assert(ContinuousRange(Arrayd::args(0.0, 0.3), true).intersects(ContinuousRange(Arrayd::args(0.1 + 4.0*M_PI, 0.3), true)));
	assert(!ContinuousRange(Arrayd::args(0.0, 0.3), true).intersects(ContinuousRange(Arrayd::args(0.4 + 4.0*M_PI, 0.31), true)));
	assert(ContinuousRange(Arrayd::args(0.0, 0.3), true).intersects(ContinuousRange(Arrayd::args(0.0, 0.3), true)));
	PASSED;
}

void example007()
{
	SysEnv env;
	Array<Nav> navs = loadNavsFromText(env.dataset.cat("allnavs.txt").str(), false);
	Array<Array<Nav> > splitNavs = splitNavsByDuration(navs, Duration::minutes(10).getDurationSeconds());
	Array<NavBBox> boxes = calcNavBBoxes(splitNavs);

	int count = boxes.size();
	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < count; j++)
		{
			assert((i == j) == (boxes[i].intersects(boxes[j])));
		}
	}
	PASSED;
}

void example008()
{
	SysEnv env;
	Array<Nav> allNavs = loadNavsFromText(env.dataset.cat("allnavs.txt").str(), false);
	Array<Array<Nav> > splitNavs = splitNavsByDuration(allNavs, Duration::minutes(10).getDurationSeconds());
	Array<Nav> navs = splitNavs.first();

	double spaceStep = 500; // metres
	double timeStep = Duration::minutes(10).getDurationSeconds();
	LocalRace race(navs, spaceStep, timeStep);
	race.makeSpatioTemporalPlot(navs);
}

void example009()
{
	ParetoFrontier frontier;
	assert(frontier.insert(ParetoElement(1.0, 1.0)));
	assert(frontier.size() == 1);
	assert(!frontier.insert(ParetoElement(2, 2)));
	assert(frontier.insert(ParetoElement(1.1, 0.9)));
	assert(frontier.size() == 2);
	assert(!frontier.insert(ParetoElement(2, 2)));
	assert(frontier.insert(ParetoElement(0.0, 0.0)));
	assert(frontier.size() == 1);
	PASSED;
}

void example010()
{

	double lower[2] = {-0.1, -0.1};
	double upper[2] = {1.2, 1.2};

	double x[2] = {0.6, 0.6};
	double spacing[2] = {1.0, 1.0};

	BBox2d box;
	box.extend(lower);
	box.extend(upper);

	Grid2d grid(box, spacing);
	//DOUT(grid);

	const int vdim = grid.WVL;
	int inds[vdim];
	double weights[vdim];
	grid.makeVertexLinearCombination(x, inds, weights);

	double wsum = 0.0;
	for (int i = 0; i < vdim; i++)
	{
		//cout << "Vertex " << inds[i] << " weighted by " << weights[i] << endl;
		wsum += weights[i];
	}
	assert(std::abs(wsum - 1) <= 1.0e-6);

	double y[2];
	grid.evalVertexLinearCombination(inds, weights, y);
	//DOUT(y[0]);
	//DOUT(y[1]);
	assert((norm2dif<double, 2>(x, y) <= 1.0e-6));
	PASSED;
}

void example011()
{
	SysEnv env;
	Array<Nav> allNavs = loadNavsFromText(env.dataset.cat("allnavs.txt").str(), false);
	Array<Array<Nav> > splitNavs = splitNavsByDuration(allNavs, Duration::minutes(10).getDurationSeconds());
	Array<Nav> navs = splitNavs.first();

	double spaceStep = 500; // metres
	double timeStep = Duration::minutes(10).getDurationSeconds();
	LocalRace race(navs, spaceStep, timeStep);
	Grid3d wind = race.getWindGrid();
	for (int i = 0; i < 3; i++)
	{
		arma::sp_mat reg = wind.makeFirstOrderReg(i);
		assert(reg.n_cols == wind.getVertexCount());
		arma::mat X = arma::ones(reg.n_cols, 1);
		double err = arma::norm(reg*X, 2);
		assert(err <= 1.0e-6);
	}
	PASSED;
}

void makeEx012NoisySignal(int sampleCount, Arrayd &X, Arrayd &Ygt, Arrayd &Ynoisy)
{
	Uniform rng(-1.0, 1.0);
	X.create(sampleCount);
	Ygt.create(sampleCount);
	Ynoisy.create(sampleCount);
	Uniform noise(-0.2, 0.2);

	std::vector<double> Xsorted(sampleCount);
	for (int i = 0; i < sampleCount; i++)
	{
		Xsorted[i] = rng.gen();
	}
	std::sort(Xsorted.begin(), Xsorted.end());


	for (int i = 0; i < sampleCount; i++)
	{
		X[i] = Xsorted[i];
		Ygt[i] = (X[i] < 0? -1.0 : 1.0);
		Ynoisy[i] = Ygt[i] + noise.gen();
	}
}


class Ex012Function : public AutoDiffFunction
{
public:
	Ex012Function(Arrayd X, Arrayd Y);

	int inDims();
	int outDims();
	void evalAD(adouble *Xin, adouble *Fout);
private:
	Arrayd _X, _Y;
};

Ex012Function::Ex012Function(Arrayd X, Arrayd Y) : _X(X), _Y(Y)
{
	assert(X.size() == Y.size());
}

int Ex012Function::inDims()
{
	return 1;
}

int Ex012Function::outDims()
{
	return _X.size();
}

void Ex012Function::evalAD(adouble *Xin, adouble *Fout)
{
	adouble &x = Xin[0];
	for (int i = 0; i < _X.size(); i++)
	{
		Fout[i] = _Y[i] + x*pow(_X[i], 3);
	}
}


void example012()
{
	BBox1d bbox(Span(-1.0, 1.0));
	double spacing[1] = {0.03};
	Grid1d grid(bbox, spacing);

	arma::sp_mat A = grid.makeFirstOrderReg(0);
	//arma::mat Adense = MAKEDENSE(A);
	//DOUT(Adense);

	int sampleCount = 30;
	Arrayd X, Ygt, Ynoisy;
	makeEx012NoisySignal(sampleCount, X, Ygt, Ynoisy);
//	GnuplotExtra plot;
//	plot.plot_xy(X, Ynoisy);
//	plot.show();

	arma::sp_mat P = grid.makeP(MDArray2d(X));

	{ // Validate P
		MDArray2d V = grid.getGridVertexCoords();
		arma::mat Vmat(V.getData(), V.rows(), V.cols(), false, true);
		arma::mat Xmat(X.getData(), X.size(), 1, false, true);
		assert(arma::norm(P*Vmat - Xmat, 2) <= 1.0e-6);
	}

	Array<Arrayb> splits = makeRandomSplits(3, X.size());

	Ex012Function data(X, Ynoisy);

	GridFitter gridFitter;


	std::shared_ptr<GridFit> gf(new GridFit(P, &data, Array<arma::sp_mat>::args(A), splits, Arrayd::args(10.0)));
	gridFitter.add(gf);

	arma::mat resmat = gf->makeDataToResidualsMat();

	arma::mat Pinv = gf->makeDataToParamMat();
	arma::mat cvfit = gf->makeCrossValidationFitnessMat();

	//DOUT(resmat);


	arma::mat params(1, 1);
	params[0] = 30.0;
	gridFitter.solveFixedReg(params);
	DOUT(params);
	std::cout << "Done" << std::endl;

	Arrayd Yfitted(sampleCount);
	data.eval(params.memptr(), Yfitted.getData());
	arma::mat vertices = Pinv*arma::mat(Yfitted.getData(), Yfitted.size(), 1, false, true);

	GnuplotExtra plot;
	plot.set_style("lines");
	plot.plot_xy(X, Ynoisy, "Noisy input");
	plot.plot_xy(X, Yfitted, "Non-linear transformation of noisy signal");
	plot.set_style("linespoints");
	plot.plot_xy(grid.getGridVertexCoords().getStorage(), Arrayd(vertices.n_elem, vertices.memptr()), "Fitted grid");
	plot.show();
}



void example013()
{
	BBox1d bbox(Span(-1.0, 1.0));
	double spacing[1] = {0.03};
	Grid1d grid(bbox, spacing);

	arma::sp_mat A = grid.makeFirstOrderReg(0);
	//arma::mat Adense = MAKEDENSE(A);
	//DOUT(Adense);

	int sampleCount = 30;
	Arrayd X, Ygt, Ynoisy;
	makeEx012NoisySignal(sampleCount, X, Ygt, Ynoisy);
//	GnuplotExtra plot;
//	plot.plot_xy(X, Ynoisy);
//	plot.show();

	arma::sp_mat P = grid.makeP(MDArray2d(X));

	{ // Validate P
		MDArray2d V = grid.getGridVertexCoords();
		arma::mat Vmat(V.getData(), V.rows(), V.cols(), false, true);
		arma::mat Xmat(X.getData(), X.size(), 1, false, true);
		assert(arma::norm(P*Vmat - Xmat, 2) <= 1.0e-6);
	}

	Array<Arrayb> splits = makeRandomSplits(9, X.size());

	Ex012Function data(X, Ynoisy);

	GridFitter gridFitter;


	double initReg = 1.0; // works
	//double initReg = 1000.0;

	std::shared_ptr<GridFit> gf(new GridFit(P, &data, Array<arma::sp_mat>::args(A), splits, Arrayd::args(initReg)));
	gridFitter.add(gf);

	arma::mat params(1, 1);
	params[0] = 3000.0;
	gridFitter.solve(params);


	arma::mat resmat = gf->makeDataToResidualsMat();
	arma::mat Pinv = gf->makeDataToParamMat();
	arma::mat cvfit = gf->makeCrossValidationFitnessMat();


	Arrayd Yfitted(sampleCount);
	data.eval(params.memptr(), Yfitted.getData());
	arma::mat D(Yfitted.getData(), Yfitted.size(), 1, false, true);
	arma::mat vertices = Pinv*D;
	DOUT(gf->getRegWeight(0));

	GnuplotExtra plot;
	plot.set_style("lines");
	plot.plot_xy(X, Ynoisy, "Noisy input");
	plot.plot_xy(X, Yfitted, "Non-linear transformation of noisy signal");
	plot.set_style("linespoints");
	plot.plot_xy(grid.getGridVertexCoords().getStorage(), Arrayd(vertices.n_elem, vertices.memptr()), "Fitted grid");
	plot.show();
}


void example014()
{
	BBox1d bbox(Span(-1.0, 1.0));
	double spacing[1] = {0.03};
	Grid1d grid(bbox, spacing);

	arma::sp_mat A1 = grid.makeFirstOrderReg(0);
	arma::sp_mat A2 = grid.makeSecondOrderReg(0);

	//arma::mat Adense = MAKEDENSE(A);
	//DOUT(Adense);

	int sampleCount = 30;
	Arrayd X, Ygt, Ynoisy;
	makeEx012NoisySignal(sampleCount, X, Ygt, Ynoisy);

	arma::sp_mat P = grid.makeP(MDArray2d(X));

	Array<Arrayb> splits = makeRandomSplits(9, X.size());

	//double initReg = 0.01; // works
	//double initReg = 1; // works
	double initReg = 0.1;


	Ex012Function data(X, Ynoisy);
	GridFitter gridFitter;
	std::shared_ptr<GridFit> gf(new GridFit(P, &data, Array<arma::sp_mat>::args(A1, A2), splits, Arrayd::args(initReg, initReg)));
	gridFitter.add(gf);

	arma::mat params(1, 1);
	params[0] = 3000.0;
	gridFitter.solve(params);

	arma::mat resmat = gf->makeDataToResidualsMat();
	arma::mat Pinv = gf->makeDataToParamMat();
	arma::mat cvfit = gf->makeCrossValidationFitnessMat();

	Arrayd Yfitted(sampleCount);
	data.eval(params.memptr(), Yfitted.getData());
	arma::mat D(Yfitted.getData(), Yfitted.size(), 1, false, true);
	arma::mat vertices = Pinv*D;

	GnuplotExtra plot;
	plot.set_style("lines");
	plot.plot_xy(X, Ynoisy, "Ground-truth noisy data vector (with correct calibration)");
	plot.plot_xy(X, Yfitted, "Data vector with estimated calibration");
	plot.set_style("linespoints");
	plot.plot_xy(grid.getGridVertexCoords().getStorage(), Arrayd(vertices.n_elem, vertices.memptr()), "Fitted model grid");
	plot.show();
}




} /* namespace sail */
