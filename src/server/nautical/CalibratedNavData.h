/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CALIBRATEDNAVDATA_H_
#define CALIBRATEDNAVDATA_H_

#include <server/nautical/CalibrationModel.h>
#include <server/nautical/FilteredNavData.h>
#include <adolc/adouble.h>
#include <server/math/nonlinear/LevmarSettings.h>


namespace sail {

class CalibratedNavData {
 public:
  class Settings {
   public:
    LevmarSettings levmar;

    Settings();

    enum CostType {L2_COST, L1_COST, COST_TYPE_COUNT};
    CostType costType;


    enum WeightType {DIRECT, SQRT_ABS, UNIFORM, WEIGHT_TYPE_COUNT};
    WeightType weightType;

    static const char *costTypeString(CostType costType);
    static const char *weightTypeString(WeightType weightType);
  };


  CalibratedNavData() : _cost(1.0e20), _initCost(1.0e20),
      _costType(Settings::COST_TYPE_COUNT), _weightType(Settings::WEIGHT_TYPE_COUNT) {}
  CalibratedNavData(FilteredNavData filteredData,
      Arrayd times = Arrayd(),
      CorrectorSet<adouble>::Ptr correctorSet =
                CorrectorSet<adouble>::Ptr(),
               Settings settings = Settings(),
               Arrayd initialization = Arrayd());

  const Arrayd optimalCalibrationParameters() const {
    return _optimalCalibrationParameters;
  }

  static Arrayd sampleTimes(FilteredNavData navdata, int count);
  static Arrayd makeAllTimes(FilteredNavData navdata);

  /*
   * Initialize from different starting points to
   * increase the chance of optaining a local optimum
   * close to the real optimum.
   */
  static CalibratedNavData bestOfInits(Array<Arrayd> initializations,
      FilteredNavData fdata, Arrayd times = Arrayd(),
      CorrectorSet<adouble>::Ptr correctorSet =
                CorrectorSet<adouble>::Ptr(),
               Settings settings = Settings());


  static CalibratedNavData bestOfInits(int initCount,
      FilteredNavData fdata, Arrayd times = Arrayd(),
      CorrectorSet<adouble>::Ptr correctorSet =
                CorrectorSet<adouble>::Ptr(),
               Settings settings = Settings());

  bool operator< (const CalibratedNavData &other) const {
    return _cost < other._cost;
  }

  double cost() const {
    return _cost;
  }

  double initCost() const {
    return _initCost;
  }

  void outputGeneralInfo(std::ostream *dst) const;

  CalibratedValues<double> calibratedValues(double localTime) const;
 private:
  double _cost, _initCost;
  FilteredNavData _filteredRawData;
  Arrayd _initialCalibrationParameters,
    _optimalCalibrationParameters;
  CorrectorSet<adouble>::Ptr _correctorSet;
  CorrectorSet<double>::Ptr _correctorSetd;
  Settings::CostType _costType;
  Settings::WeightType _weightType;
};

}

#endif /* CALIBRATEDNAVDATA_H_ */
