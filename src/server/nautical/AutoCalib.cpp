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
    class WindAndCurrentDifs {
     public:
      WindAndCurrentDifs(HorizontalMotion<T> wd, HorizontalMotion<T> cd) :
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
    WindAndCurrentDifs<T> calcWindAndCurrentDifs(const Corrector<T> &corrector, int timeIndex) const {
      int lowerIndex = calcLowerIndex(timeIndex);
      int upperIndex = lowerIndex + 1;

      CalibratedNav<T> a = corrector.correct(FilteredNavInstrumentAbstraction(_data, lowerIndex));
      CalibratedNav<T> b = corrector.correct(FilteredNavInstrumentAbstraction(_data, upperIndex));

      HorizontalMotion<T> wdif = b.trueWind() - a.trueWind();
      HorizontalMotion<T> cdif = b.trueCurrent() - a.trueCurrent();
      T factor = T(1.0/_data.samplingPeriod());
      return WindAndCurrentDifs<T>(factor*wdif, factor*cdif);
    }

    Vectorize<double, 2> evalSub(int index, const double *X, double *F, double *J,
        int *windInlierCounter, int *currentInlierCounter) const;


    void computeWindAndCurrentDerivNorms(int classIndex,
        Array<ResidueData> *Wdst, Array<ResidueData> *Cdst);
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

      WindAndCurrentDifs<adouble> difs = calcWindAndCurrentDifs<adouble>(*corr, index);
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


  void Objf::computeWindAndCurrentDerivNorms(int classIndex,
      Array<ResidueData> *Wdst, Array<ResidueData> *Cdst) {
    int windInlierCounter = 0;
    int currentInlierCounter = 0;

    int count = length();
    Corrector<double> corr;
    double *X = corr.toArray().ptr();
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
    computeWindAndCurrentDerivNorms(-1, &W, &C);

    SCOPEDMESSAGE(INFO, "Compute the wind quality parameter");
    _qualityWind = computeParam(W, _settings.wind);
    SCOPEDMESSAGE(INFO, "Compute the current quality parameter");
    _qualityCurrent = computeParam(C, _settings.current);
    SCOPEDMESSAGE(INFO, stringFormat("     Wind quality parameter set to %.3g", _qualityWind));
    SCOPEDMESSAGE(INFO, stringFormat("  Current quality parameter set to %.3g", _qualityCurrent));
  }





  class OptQuality {
   public:
    OptQuality() : quality(NAN), objfValue(std::numeric_limits<double>::infinity()) {}
    OptQuality(double v, double q) : objfValue(v), quality(q) {}

    double quality;

    bool operator< (const OptQuality &other) const {
      return objfValue < other.objfValue;
    }

    double objfValue;
  };

  double calcMatchValue(int inlierCounters[2], int matchCounter) {
    if (matchCounter == 0) {
      return 1.0e9;
    }

    return double(sqr(inlierCounters[0]) + sqr(inlierCounters[1]))
                    /matchCounter;
  }

  double calcMatchValue2(int positiveMatches, int negativeMatches, double expt = 1.0) {
    if (positiveMatches == 0 || negativeMatches == 0) {
      return 1.0e6;
    }
    return std::pow(1.0/positiveMatches, expt) + std::pow(1.0/negativeMatches, expt);
  }

  double calcMatchValue3(int inliers[2], int matches) {
    return double(sqr(inliers[0]) + sqr(inliers[1]) + 3)/matches;
  }

  double calcMatchValue4(int inliers[2], int inlierMatches) {
    double lambda = 0.08;
    return -inlierMatches + lambda*(sqr(inliers[0]) + sqr(inliers[1]));
  }

  OptQuality optimizeQualityParameter(
      Array<ResidueData> residuesA,
      Array<ResidueData> residuesB) {
    int count = residuesA.size();
    assert(count == residuesB.size());
    for (int i = 0; i < count; i++) {
      assert(residuesA[i].classIndex() == 0);
      assert(residuesB[i].classIndex() == 1);
    }

    Array<ResidueData> allResidues(2*count);
    residuesA.copyToSafe(allResidues.sliceTo(count));
    residuesB.copyToSafe(allResidues.sliceFrom(count));
    sort(allResidues);

    Arrayb inliers[2];
    for (int i = 0; i < 2; i++) {
      inliers[i] = Arrayb::fill(count, false);
    }
    int inlierCounters[2] = {0, 0};
    int inlierMatchCounter = 0;
    int mismatchCounter = 0;
    Array<std::pair<double, double> > scorePerThreshold;

    OptQuality best;
    int totalCount = 2*count;


    Arrayd X(totalCount), Y(totalCount);

    for (int i = 0; i < totalCount; i++) {
      ResidueData &x = allResidues[i];
      inliers[x.classIndex()][x.sampleIndex()] = true;
      inlierCounters[x.classIndex()]++;
      if (inliers[0][x.sampleIndex()] && inliers[1][x.sampleIndex()]) {
        inlierMatchCounter++;
        mismatchCounter--;
      } else {
        mismatchCounter++;
      }
      int outlierMatchCount = count - inlierMatchCounter - mismatchCounter;
      //double value = calcMatchValue(inlierCounters, matchCounter);
      double value = calcMatchValue2(inlierMatchCounter, outlierMatchCount, 1.0e-3);
      //double value = calcMatchValue3(inlierCounters, inlierMatchCounter);
      //double value = calcMatchValue4(inlierCounters, inlierMatchCounter);

      X[i] = i;
      Y[i] = value;


      best = std::min(best, OptQuality(value, x.calcThresholdQuality()));
    }


    Arrayb mask = Y.map<bool>([&](double x) {return x < 1.0e3;});
    std::cout << EXPR_AND_VAL_AS_STRING(mask) << std::endl;
    GnuplotExtra plot;
    plot.set_style("lines");
    plot.plot_xy(X.slice(mask), Y.slice(mask));
    plot.show();

    return best;
  }
}

AutoCalib::Results AutoCalib::calibrate(FilteredNavData data, Arrayd times) const {
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
  auto cost = new Objf(data, times, _settings);
  Corrector<double> corr;
  problem.AddResidualBlock(cost, NULL, (double *)(&corr));
  ceres::Solver::Options options;
  options.minimizer_progress_to_stdout = true;
  ceres::Solver::Summary summary;
  Solve(options, &problem, &summary);
  SCOPEDMESSAGE(INFO, "Done optimizing.");
  return Results(corr, data);
}

AutoCalib::Results AutoCalib::calibrateAutotune(FilteredNavData data,
    Arrayd times, Arrayb split) const {
  Objf A(data, times.slice(split), _settings);
  Objf B(data, times.slice(neg(split)), _settings);
  return AutoCalib::Results();
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







namespace {

  void runOptimalQualityTest3() {
    std::default_random_engine e;
    int count = 30;

    const int tau = 12;
    Array<ResidueData> dataA(count), dataB(count);
    for (int i = 0; i < count; i++) {
      double offset = (i < tau? 0 : 2);
      std::uniform_real_distribution<double> distrib(offset + 0, offset + 1);
      dataA[i] = ResidueData(i, 1.0, distrib(e), 0);
      dataB[i] = ResidueData(i, 1.0, distrib(e), 1);
    }
    OptQuality opt = optimizeQualityParameter(dataA, dataB);

    double gtQuality = 0.5*(dataA[tau-1].calcThresholdQuality() + dataA[tau].calcSquaredThresholdQuality());
    //opt.quality =
    for (int i = 0; i < count; i++) {
      std::cout << "i = " << i << "  A: " << dataA[i].isInlier(opt.quality) <<
          "    B: " << dataB[i].isInlier(opt.quality) << std::endl;
    }
  }



  void runOptimalQualityTest4() {
    std::default_random_engine e;
    int count = 30;

    const int tau = 7;
    Array<ResidueData> dataA(count), dataB(count);
    for (int i = 0; i < count; i++) {
      double offset = (i < tau? 0 : 1.8);
      std::uniform_real_distribution<double> distrib(offset + 0, offset + 2);
      dataA[i] = ResidueData(i, 1.0, distrib(e), 0);
      dataB[i] = ResidueData(i, 1.0, distrib(e), 1);
    }

    dataA[0] = ResidueData(0, 1.0, 100.0, 0);


    OptQuality opt = optimizeQualityParameter(dataA, dataB);

    double gtQuality = 0.5*(dataA[tau-1].calcThresholdQuality() + dataA[tau].calcSquaredThresholdQuality());
    //opt.quality =
    for (int i = 0; i < count; i++) {
      std::cout << "i = " << i << "  A: " << dataA[i].isInlier(opt.quality) <<
          "    B: " << dataB[i].isInlier(opt.quality) << std::endl;
    }
  }

}

void runExtraAutoCalibTests() {
  //runOptimalQualityTest1();
  //runOptimalQualityTest2();
  runOptimalQualityTest4();
}


}
