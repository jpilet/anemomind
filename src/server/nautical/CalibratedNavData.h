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

  CalibratedNavData(FilteredNavData filteredData,
      Arrayd times = Arrayd(),
      CorrectorSet<adouble>::Ptr correctorSet =
                CorrectorSet<adouble>::Ptr(),
               LevmarSettings settings = LevmarSettings(),
               Arrayd initialization = Arrayd());

  const Arrayd optimalCalibrationParameters() const {
    return _optimalCalibrationParameters;
  }

  static Arrayd sampleTimes(FilteredNavData navdata, int count);

  /*
   * Initialize from different starting points to
   * increase the chance of optaining a local optimum
   * close to the real optimum.
   */
  static CalibratedNavData bestOfInits(Array<Arrayd> initializations,
      FilteredNavData fdata, Arrayd times = Arrayd(),
      CorrectorSet<adouble>::Ptr correctorSet =
                CorrectorSet<adouble>::Ptr(),
               LevmarSettings settings = LevmarSettings());


  static CalibratedNavData bestOfInits(int initCount,
      FilteredNavData fdata, Arrayd times = Arrayd(),
      CorrectorSet<adouble>::Ptr correctorSet =
                CorrectorSet<adouble>::Ptr(),
               LevmarSettings settings = LevmarSettings());

  bool operator< (const CalibratedNavData &other) const {
    return _value < other._value;
  }
 private:
  double _value;
  FilteredNavData _filteredRawData;
  Arrayd _initialCalibrationParameters,
    _optimalCalibrationParameters;
  CorrectorSet<adouble>::Ptr _correctorSet;
};

}

#endif /* CALIBRATEDNAVDATA_H_ */
