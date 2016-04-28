/*
 * CurrentCalib.h
 *
 *  Created on: Apr 15, 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_CALIB_CURRENTCALIB_H_
#define SERVER_NAUTICAL_CALIB_CURRENTCALIB_H_

#include <Eigen/Dense>
#include <server/nautical/calib/CornerCalib.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/nautical/NavDataset.h>

namespace sail {
namespace CurrentCalib {

struct Settings {
  Settings();

  Duration<double> samplingPeriod;
  Duration<double> windowSize;

  bool debugShowScatter;
};

typedef Array<TimedValue<HorizontalMotion<double> > > MotionSamples;

Array<MotionSamples> computeCorrectedCurrent(
    const Array<NavDataset> &ds, const Settings &s);

}
}


#endif /* SERVER_NAUTICAL_CALIB_CURRENTCALIB_H_ */
