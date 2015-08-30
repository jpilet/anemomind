/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_CALIBRATION_FULLCALIBRATION_H_
#define SERVER_NAUTICAL_CALIBRATION_FULLCALIBRATION_H_

namespace sail {
namespace FullCalibration {

struct Settings {
 bool offsetWind, offsetCurrent;
 int order;
};

}
}

#endif /* SERVER_NAUTICAL_CALIBRATION_FULLCALIBRATION_H_ */
