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
      CorrectorSet<adouble>::Ptr correctorSet =
          CorrectorSet<adouble>::Ptr(),
          Arrayd times = Arrayd(),
          LevmarSettings settings = LevmarSettings());

  const Arrayd optimalCalibrationParameters() const {
    return _optimalCalibrationParameters;
  }

 private:
  FilteredNavData _filteredRawData;
  Arrayd _optimalCalibrationParameters;
  CorrectorSet<adouble>::Ptr _correctorSet;
};

}

#endif /* CALIBRATEDNAVDATA_H_ */
