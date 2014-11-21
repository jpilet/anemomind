/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <ceres/ceres.h>
#include <server/math/armaadolc.h>
#include <server/common/Function.h>
#include <server/nautical/AutoCalib.h>
#include <adolc/taping.h>
#include <server/common/ScopedLog.h>
#include <server/common/PhysicalQuantityIO.h>
#include <adolc/drivers/drivers.h>
#include <ceres/fpclassify.h>
#include <random>
#include <server/plot/extra.h>
#include <server/common/ArrayIO.h>
#include <server/math/nonlinear/Levmar.h>
#include <server/math/nonlinear/LevmarSettings.h>
#include <server/common/MeanAndVar.h>
#include <server/math/nonlinear/Multiplayer.h>
#include <server/math/nonlinear/StepMinimizer.h>
#include <server/common/SharedPtrUtils.h>

namespace ceres {
namespace internal {
  bool IsArrayValid(int size, const double* x);
}
}


namespace sail {

namespace {
  typedef AutoCalib::Settings::QParam QParam;

  class ResidueData {
   public:
    ResidueData() : _g(NAN), _residue(NAN),
      _index(-1), _classIndex(-1) {}

    ResidueData(int index, double g, double x, int classIndex_) :
      _g(g), _residue(x), _index(index),
      _classIndex(classIndex_) {}

    bool isInlier(double quality) const {
      return sqr(quality*_residue) <= _g;
    }

    bool operator< (const ResidueData &other) const {
      return calcSquaredThresholdQuality() > other.calcSquaredThresholdQuality();
    }


    double calcSquaredThresholdQuality() const {
      return _g/sqr(_residue);
    }

    // The quality setting at which this residues shifts
    // between being an inlier or an outlier.
    double calcThresholdQuality() const {
      return sqrt(calcSquaredThresholdQuality());
    }

    const int sampleIndex() const {
      return _index;
    }

    const int classIndex() const {
      return _classIndex;
    }
   private:
    int _index;
    int _classIndex;
    double _g, _residue;
  };

  template <typename T>
  T makePositive(T x) {
    constexpr double minv = 1.0e-4;
    if (x < minv) {
      return minv;
    }
    return x;
  }

  template <typename T>
  Velocity<T> makePositive(Velocity<T> x) {
    return Velocity<T>::knots(makePositive(x.knots()));
  }

  class FilteredNavInstrumentAbstraction {
   public:
    FilteredNavInstrumentAbstraction(const FilteredNavData &data, int index) :
      _data(data), _index(index) {}

    Angle<double> awa() const {
      return _data.awa().get(_index);
    }

    Angle<double> magHdg() const {
      return _data.magHdg().get(_index);
    }

    Velocity<double> aws() const {
      return makePositive(_data.aws().get(_index));
    }

    Velocity<double> watSpeed() const {
      return makePositive(_data.watSpeed().get(_index));
    }

    Angle<double> gpsBearing() const {
      return _data.gpsBearing().get(_index);
    }

    Velocity<double> gpsSpeed() const {
      return _data.gpsSpeed().get(_index);
    }
   private:
    const FilteredNavData &_data;
    int _index;
  };

  class Objf : public ceres::CostFunction {
   public:
    static constexpr int blockSize = 6;
    Objf(FilteredNavData data, Arrayd times, AutoCalib::Settings s);

    template <typename T>
    class WindAndCurrentDataDifs {
     public:
      WindAndCurrentDataDifs(HorizontalMotion<T> wd,
                         HorizontalMotion<T> cd) :
        windDif(wd), currentDif(cd) {}
      HorizontalMotion<T> windDif;
      HorizontalMotion<T> currentDif;
    };

    int length() const {
      return _times.size();
    }

    int inDims() const {
      return Corrector<double>::paramCount();
    }

    int outDims() const {
      return blockSize*length();
    }


    //http://ceres-solver.org/modeling.html#CostFunction::Evaluate__doubleCPCP.doubleP.doublePP
    bool Evaluate(double const *const *parameters,
                          double *residuals,
                          double **jacobians) const {
      //CostFunction::parameter_block_sizes_
      assert(parameter_block_sizes().size() == 1);
      const double *psub = parameters[0];
      double *j = (jacobians == nullptr? nullptr : jacobians[0]);
      eval(psub, residuals, j);
      return true;
    }

    void computeWindAndCurrentDerivNorms(Corrector<double> *corr,
        int classIndex,
        Array<ResidueData> *Wdst, Array<ResidueData> *Cdst);

    void setQualityWind(double qw) {
      _qualityWind = qw;
    }

    void setQualityCurrent(double qc) {
      _qualityCurrent = qc;
    }

    double qualityWind() const {
      return _qualityWind;
    }

    double qualityCurrent() const {
      return _qualityCurrent;
    }
   private:
    void eval(const double *X, double *F, double *J) const;
    AutoCalib::Settings _settings;
    FilteredNavData _data;
    Arrayd _times, _rateOfChange;
    Arrayd _tempW, _tempC;
    double _qualityWind, _qualityCurrent;

    int calcLowerIndex(int timeIndex) const {
      return int(floor(_data.sampling().inv(_times[timeIndex])));
    }

    double normGDeriv(int timeIndex) const;

    template <typename T>
    WindAndCurrentDataDifs<T> calcWindAndCurrentDifs(const Corrector<T> &corrector, int timeIndex) const {
      int lowerIndex = calcLowerIndex(timeIndex);
      int upperIndex = lowerIndex + 1;

      CalibratedNav<T> a = corrector.correct(FilteredNavInstrumentAbstraction(_data, lowerIndex));
      CalibratedNav<T> b = corrector.correct(FilteredNavInstrumentAbstraction(_data, upperIndex));

      HorizontalMotion<T> wdif = b.trueWind() - a.trueWind();
      HorizontalMotion<T> cdif = b.trueCurrent() - a.trueCurrent();
      T factor = T(1.0/_data.samplingPeriod());
      return WindAndCurrentDataDifs<T>(factor*wdif, factor*cdif);
    }

    Vectorize<double, 2> evalSub(int index, const double *X, double *F, double *J,
        int *windInlierCounter, int *currentInlierCounter) const;


    void tuneParameters();
  };



  bool isOK(int rows, int cols, const double *X) {
    return ceres::internal::IsArrayValid(rows*cols, X);
  }

  template <typename T>
  T unwrap(Velocity<T> x) {
    return x.knots();
  }

  double calcSigma(double g, double quality) {
    return sqrt(g/sqr(quality));
  }

  Objf::Objf(FilteredNavData data, Arrayd times, AutoCalib::Settings s) :
      _data(data), _times(times), _qualityWind(s.wind.fixedQuality),
      _qualityCurrent(s.current.fixedQuality), _settings(s), _rateOfChange(times.size()) {
      for (int i = 0; i < length(); i++) {
        _rateOfChange[i] = normGDeriv(i);
      }

      tuneParameters();

      { // ceres-related stuff.
        mutable_parameter_block_sizes()->resize(1);
        (*mutable_parameter_block_sizes())[0] = inDims();
        set_num_residuals(outDims());
      }
  }

  double Objf::normGDeriv(int timeIndex) const {
    int lowerIndex = calcLowerIndex(timeIndex);
    if (_settings.gMode == AutoCalib::Settings::GPS) {
      HorizontalMotion<double> a = _data.gpsMotionAtIndex(lowerIndex + 0);
      HorizontalMotion<double> b = _data.gpsMotionAtIndex(lowerIndex + 1);
      auto dif = b - a;
      return sqrt(sqr(dif[0].knots()) + sqr(dif[1].knots()))/_data.samplingPeriod();
    } else {
      return std::abs(_data.magHdg().interpolateLinearDerivative(
          _data.sampling()(_times[timeIndex])).degrees());
    }
  }

  void Objf::eval(const double *X, double *F, double *J) const {
    ENTERSCOPE("Evaluate the automatic calibration objective function");
    bool outputJ = J != nullptr;

    int windInlierCounter = 0;
    int currentInlierCounter = 0;
    int jstep = blockSize*inDims();
    for (int i = 0; i < length(); i++) {
      evalSub(i, X, F + blockSize*i, (outputJ? J + i*jstep : nullptr),
        &windInlierCounter, &currentInlierCounter);
    }
    assert(isOK(outDims(), 1, F));
    if (J != nullptr) {
      assert(isOK(outDims(), inDims(), J));
    }
    assert(outDims() == this->num_residuals());
    assert(inDims()  == this->parameter_block_sizes()[0]);
    assert(1         == this->parameter_block_sizes().size());
    SCOPEDMESSAGE(INFO, stringFormat("Wind inlier count:    %d", windInlierCounter));
    SCOPEDMESSAGE(INFO, stringFormat("Current inlier count: %d", currentInlierCounter));
    SCOPEDMESSAGE(INFO, stringFormat("Output Jacobian:      %s", (J == nullptr? "NO" : "YES")));
  }


  double evalRobust(bool smooth, double quality,
      const HorizontalMotion<adouble> &X, double g, adouble *result,
      int *inlierCounter) {
    double sigma = calcSigma(g, quality);
    adouble x = X.norm().knots();
    double xd = x.getValue();
    bool inlier = (xd < sigma);
    adouble lambda = (smooth?
                        sqr(sigma)/(sqr(sigma) + sqr(x))
                        : adouble(inlier? 1 : 0));
    if (std::isnan(lambda.getValue())) { // Could occur if smooth=true, g=0 and x=0
      lambda = 0;
    }
    adouble ql = quality*lambda;
    result[0] = ql*X[0].knots();
    result[1] = ql*X[1].knots();
    result[2] = quality*(1.0 - lambda)*sigma;
    if (inlier) {
      (*inlierCounter)++;
    }
    return xd;
  }



  Vectorize<double, 2> Objf::evalSub(int index, const double *X, double *F, double *J,
      int *windInlierCounter, int *currentInlierCounter) const {
    double g = _rateOfChange[index];
    bool outputJ = J != nullptr;

    if (outputJ) {
      trace_on(_settings.tapeIndex);
    }
    Array<adouble> adX = adolcInput(inDims(), X);
    Corrector<adouble> *corr = Corrector<adouble>::fromArray(adX);
    adouble result[blockSize];

      WindAndCurrentDataDifs<adouble> difs = calcWindAndCurrentDifs<adouble>(*corr, index);
      double w = evalRobust(_settings.smooth, _qualityWind, difs.windDif,    g, result + 0,
        windInlierCounter);
      double c = evalRobust(_settings.smooth, _qualityCurrent, difs.currentDif, g, result + 3,
        currentInlierCounter);

    adolcOutput(blockSize, result, F);
    if (outputJ) {
      trace_off();
      outputJacobianRowMajor(_settings.tapeIndex, X, J);
      assert(isOK(blockSize, inDims(), J));
    }
    assert(isOK(blockSize, 1, F));
    return Vectorize<double, 2>{w, c};
  }



  int countInliers(Array<ResidueData> X, double q) {
    for (int i = 0; i < X.size(); i++) {
      if (!X[i].isInlier(q)) {
        return i;
      }
    }
    return X.size();
  }



  double tuneParam(Array<ResidueData> X, QParam qsettings) {
    ENTERSCOPE("Tune a quality parameter");
    if (X.size() < qsettings.minCount) {
      LOG(FATAL) << "Too few measurements to perform accurate calibration";
    }
    int desiredCount = qsettings.minCount + int(floor(qsettings.frac*(X.size() - qsettings.minCount)));
    SCOPEDMESSAGE(INFO, stringFormat("Tune the quality parameter so that %d of the %d measurements are inliers. (%.3g percents)",
        desiredCount, X.size(), (100.0*desiredCount)/X.size()));
    return X[desiredCount-1].calcThresholdQuality();
  }

  double computeParam(Array<ResidueData> X, QParam qsettings) {
    if (qsettings.mode == QParam::FIXED ||
        qsettings.mode == QParam::TUNE_ON_ERROR) {
      int count = countInliers(X, qsettings.fixedQuality);
      if (count < qsettings.minCount) {
        if (qsettings.mode == QParam::FIXED) {
          LOG(FATAL) << "Too few inliers. Consider setting mode to TUNED or TUNE_ON_ERROR";
          return NAN;
        } else {
          return tuneParam(X, qsettings);
        }
      }
      return qsettings.fixedQuality;
    } else {
      return tuneParam(X, qsettings);
    }
  }

  template <typename T>
  void sort(Array<T> X) {
    std::sort(X.begin(), X.end());
  }


  void Objf::computeWindAndCurrentDerivNorms(Corrector<double> *corr,
      int classIndex,
      Array<ResidueData> *Wdst, Array<ResidueData> *Cdst) {
    int windInlierCounter = 0;
    int currentInlierCounter = 0;

    int count = length();
    double *X = (double *)corr;
    double temp[blockSize];
    Array<ResidueData> W(count), C(count);
    for (int i = 0; i < count; i++) {
      double g = _rateOfChange[i];
      Vectorize<double, 2> x = evalSub(i, X, temp, nullptr,
          &windInlierCounter, &currentInlierCounter);
      W[i] = ResidueData(i, g, x[0], classIndex);
      C[i] = ResidueData(i, g, x[1], classIndex);
    }
    sort(W);
    sort(C);
    *Wdst = W;
    *Cdst = C;
  }

  void Objf::tuneParameters() {
    ENTERSCOPE("Tune the quality parameters");
    _qualityWind = 1;
    _qualityCurrent = 1;

    Array<ResidueData> W, C;
    Corrector<double> corr;
    computeWindAndCurrentDerivNorms(&corr, -1, &W, &C);

    SCOPEDMESSAGE(INFO, "Compute the wind quality parameter");
    _qualityWind = computeParam(W, _settings.wind);
    SCOPEDMESSAGE(INFO, "Compute the current quality parameter");
    _qualityCurrent = computeParam(C, _settings.current);
    SCOPEDMESSAGE(INFO, stringFormat("     Wind quality parameter set to %.3g", _qualityWind));
    SCOPEDMESSAGE(INFO, stringFormat("  Current quality parameter set to %.3g", _qualityCurrent));
  }


}


AutoCalib::Results AutoCalib::calibrate(FilteredNavData data, Arrayd times) const {
  return calibrateSub(data, times, _settings);
}

AutoCalib::Results AutoCalib::calibrateSub(FilteredNavData data, Arrayd times,
  AutoCalib::Settings settings) const {
  if (times.empty()) {
    times = data.makeCenteredX();
  }

  ENTERSCOPE("Automatic calibration");
  SCOPEDMESSAGE(INFO, stringFormat("Number of measurements:          %d", times.size()));
  SCOPEDMESSAGE(INFO, stringFormat("Number of parameters to recover: %d", Corrector<double>::paramCount()));

  SCOPEDMESSAGE(INFO, "Perform optimization");
  ceres::Problem problem;

  // Set up the only cost function (also known as residual). This uses
  // auto-differentiation to obtain the derivative (jacobian).
  auto cost = new Objf(data, times, settings);
  Corrector<double> corr;
  problem.AddResidualBlock(cost, NULL, (double *)(&corr));
  ceres::Solver::Options options;
  options.minimizer_progress_to_stdout = true;
  ceres::Solver::Summary summary;
  Solve(options, &problem, &summary);
  SCOPEDMESSAGE(INFO, "Done optimizing.");
  return Results(corr, data);
}


namespace {


  class ObjfWrap : public Function {
   public:
    ObjfWrap(Objf &o) : _objf(o) {}
    int inDims() {return _objf.inDims();}
    int outDims() {return _objf.outDims();}
    void eval(double *Xin, double *Fout, double *Jout);
   private:
    Objf &_objf;
  };

  void ObjfWrap::eval(double *Xin, double *Fout, double *Jout) {
    if (Jout == nullptr) {
      _objf.Evaluate(&Xin, Fout, nullptr);
    } else {
      MDArray2d Jtranspose(inDims(), outDims());
      double *jdst = Jtranspose.ptr();
      _objf.Evaluate(&Xin, Fout, &(jdst));
      MDArray2d Jdst(outDims(), inDims(), Jout);
      for (int i = 0; i < Jdst.rows(); i++) {
        for (int j = 0; j < Jdst.cols(); j++) {
          Jdst(i, j) = Jtranspose(j, i);
        }
      }
    }
  }

}



namespace {

  Array<CalibratedNav<double> > correctValues(
      FilteredNavData fdata,
    Arrayd times, Corrector<double> corr) {
    int count = times.size();
    Array<CalibratedNav<double> > navs(count);
    for (int i = 0; i < count; i++) {
      int index = int(floor(fdata.sampling().inv(times[i])));
      navs[i] = corr.correct(
          FilteredNavInstrumentAbstraction(fdata, index));
    }
    return navs;
  }

  double computeVariance(Array<Array<CalibratedNav<double> > > calibs,
      std::function<HorizontalMotion<double>(const CalibratedNav<double> &)> f) {
    int setCount = calibs.size();
    int sampleCount = calibs[0].size();
    double varsum = 0;
    for (int j = 0; j < sampleCount; j++) {
      MeanAndVar x, y;
      for (int i = 0; i < setCount; i++) {
        HorizontalMotion<double> m = f(calibs[i][j]);
        x.add(m[0].knots());
        y.add(m[1].knots());
      }
      varsum += x.variance() + y.variance();
    }
    return varsum;
  }

  Vectorize<double, 2> computeTotalVariance(
      FilteredNavData data,
      Arrayd times,
      Array<Corrector<double> > corrs) {
    int count = corrs.size();
    Array<Array<CalibratedNav<double> > > calibs(count);
    for (int i = 0; i < count; i++) {
      calibs[i] = correctValues(data, times, corrs[i]);
    }

    auto w = [](CalibratedNav<double> n) {return n.trueWind();};
    auto c = [](CalibratedNav<double> n) {return n.trueCurrent();};
    return Vectorize<double, 2>{computeVariance(calibs, w),
                                computeVariance(calibs, c)};
  }

  Vectorize<double, 2> evalFitness(
      FilteredNavData data,
      Arrayd allTimes, Array<Arrayb> subsets,
      AutoCalib::Settings settings, double qw, double qc) {
    ENTER_FUNCTION_SCOPE;
    SCOPEDMESSAGE(INFO, stringFormat("qw = %.3g", qw));
    SCOPEDMESSAGE(INFO, stringFormat("qc = %.3g", qc));
    int mc = 0;
    settings.wind = QParam(QParam::FIXED, qw, mc, 0.5);
    settings.current = QParam(QParam::FIXED, qc, mc, 0.5);


    if (allTimes.empty()) {
      allTimes = data.makeCenteredX();
    }


    int setCount = subsets.size();
    Array<Corrector<double> > correctors(setCount);
    for (int i = 0; i < setCount; i++) {
      ENTERSCOPE(stringFormat("Optimize over set %d/%d", i+1, setCount));
      Arrayd times = allTimes.slice(subsets[i]);
      ceres::Problem problem;

      // Set up the only cost function (also known as residual). This uses
      // auto-differentiation to obtain the derivative (jacobian).
      auto cost = new Objf(data, times, settings);
      problem.AddResidualBlock(cost, NULL, (double *)correctors.ptr(i));
      ceres::Solver::Options options;
      ceres::Solver::Summary summary;
      Solve(options, &problem, &summary);
      SCOPEDMESSAGE(INFO, "Done optimizing.");
    }
    Vectorize<double, 2> fitness = computeTotalVariance(data, allTimes, correctors);
    SCOPEDMESSAGE(INFO, stringFormat("Wind fitness: %.3g", fitness[0]));
    SCOPEDMESSAGE(INFO, stringFormat("Current fitness: %.3g", fitness[1]));
    return fitness;
  }

  class CVObjf : public Function {
   public:
    CVObjf(FilteredNavData fdata,
    Arrayd allTimes,
    Array<Arrayb> subsets,
    AutoCalib::Settings settings,
    int dstIndex) : _fdata(fdata),
      _allTimes(allTimes), _subsets(subsets),
      _settings(settings), _dstIndex(dstIndex) {}

    int inDims() {return 2;}
    int outDims() {return 1;}
    void eval(double *Xin, double *Fout, double *Jout);
   private:
    FilteredNavData _fdata;
    Arrayd _allTimes;
    Array<Arrayb> _subsets;
    AutoCalib::Settings _settings;
    int _dstIndex;
  };

  void CVObjf::eval(double *Xin, double *Fout, double *Jout) {
    assert(Jout == nullptr);
    ENTER_FUNCTION_SCOPE;
    SCOPEDMESSAGE(INFO, stringFormat("log qw = %.3g", Xin[0]));
    SCOPEDMESSAGE(INFO, stringFormat("log qc = %.3g", Xin[1]));
    Fout[0] = evalFitness(
              _fdata,
              _allTimes, _subsets,
              _settings, exp(Xin[0]), exp(Xin[1]))[_dstIndex];
  }


  AutoCalib::Settings adjustSettingsCV(FilteredNavData data,
      Arrayd times, Array<Arrayb> subsets, AutoCalib::Settings localSettings) {
    ENTER_FUNCTION_SCOPE;
    localSettings.wind = QParam::half(20);
    localSettings.current = QParam::half(20);
    auto objf = new Objf(data, times, localSettings);

    Arrayd initSteps = Arrayd::fill(2, 0.1);
    Arrayd params = Arrayd::args(log(objf->qualityWind()), log(objf->qualityCurrent()));

    SCOPEDMESSAGE(INFO, stringFormat(
        "INITIAL PARAMETERS: log(qw) = %.3g,   log(qc) = %.3g",
        params[0], params[1]));

    StepMinimizer minimizer;

    CVObjf a(data, times, subsets, localSettings, 0);
    CVObjf b(data, times, subsets, localSettings, 1);
    Array<std::shared_ptr<Function> > funs =
        Array<std::shared_ptr<Function> >::args(makeSharedPtrToStack(a),
            makeSharedPtrToStack(b));
    Arrayd optLogQ = optimizeMultiplayer(minimizer,
        funs, params, initSteps);
    int minCount = 0;
    localSettings.wind = QParam(QParam::FIXED, exp(optLogQ[0]), minCount, 0.5);
    localSettings.current = QParam(QParam::FIXED, exp(optLogQ[1]), minCount, 0.5);
    return localSettings;
  }
}



AutoCalib::Results AutoCalib::calibrateAutotuneGame(FilteredNavData data,
    Arrayd times, Array<Arrayb> subsets) const {
  return calibrateSub(data, times, adjustSettingsCV(data, times, subsets, _settings));
}



void AutoCalib::Results::disp(std::ostream *dst) {
  if (dst == nullptr) {
    dst = &(std::cout);
  }

  int sampleCount = 12;
  LineKM sample(0, sampleCount-1, log(1.0), log(40));
  for (int i = 0; i < sampleCount; i++) {
    Velocity<double> speed = Velocity<double>::knots(exp(sample(i)));
    *dst << "  A raw AWS of " << speed << " maps to a corrected AWS of "
        << _calibratedCorrector.aws.correct(speed) << std::endl;
  }
  for (int i = 0; i < sampleCount; i++) {
    Velocity<double> speed = Velocity<double>::knots(exp(sample(i)));
    *dst << "  A raw water speed of " << speed << " maps to a corrected water speed of "
        << _calibratedCorrector.watSpeed.correct(speed) << std::endl;
  }
  Angle<double> a0 = Angle<double>::degrees(0);
  *dst << "The AWA offset is " << _calibratedCorrector.awa.correct(a0) <<std::endl;
  *dst << "The magnetic heading offset is " << _calibratedCorrector.magHdg.correct(a0) << std::endl;
  *dst << "The maximum drift angle is "
      << Angle<double>::radians(_calibratedCorrector.driftAngle.amp).degrees()
      << " degrees" << std::endl;
}

}
