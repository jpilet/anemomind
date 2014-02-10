#include <server/common/Uniform.h>
#include <server/plot/extra.h>
#include <server/math/nonlinear/GridFitter.h>
#include <server/math/Grid.h>
#include <server/math/ADFunction.h>
#include <server/common/debug.h>
#include "ExSwitch.h"

using namespace sail;

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
	gridFitter.solveFixedReg(&params);
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
	gridFitter.solve(&params);


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
	gridFitter.solve(&params);

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
	plot.plot_xy(grid.getGridVertexCoords().getStorage(),
                     Arrayd(vertices.n_elem, vertices.memptr()), "Fitted model grid");
	plot.show();
}


int main(int argc, char **argv) {
  //example014();
  ExSwitch ex;
  ADDEXAMPLE(ex, example012, "Fit signal to noisy observations with fixed regularization.");
  ADDEXAMPLE(ex, example013, "Fit signal to noisy observations with auto-tuned first-order regularization.");
  ADDEXAMPLE(ex, example014, "Fit signal to noisy observations with auto-tuned first- and second-order regularization.");
  ex.run(argc, argv);
  return 0;
}
