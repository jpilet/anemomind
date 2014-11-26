/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef PPM_H_
#define PPM_H_

#include <server/graphics/RasterImage.h>

namespace sail {

void writePPM(std::string filename, RGBByteImage image);

}

#endif /* PPM_H_ */
