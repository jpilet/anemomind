/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CALIBRATEDNAVDATA_H_
#define CALIBRATEDNAVDATA_H_

#include <server/nautical/CalibrationModel.h>


namespace sail {

class CalibratedNavData {
 public:

  CalibratedNavData(FilteredNavData filteredData,
      CorrectorSet<adouble>::Ptr correctorSet);

  const Arrayd optimalCalibrationParameters() const {
    return _optimalCalibrationParameters();
  }

 private:
  FilteredNavData _filteredRawData;
  Arrayd _optimalCalibrationParameters;
  CorrectorSet<adouble>::Ptr _correctorSet;
};

}

#endif /* CALIBRATEDNAVDATA_H_ */
