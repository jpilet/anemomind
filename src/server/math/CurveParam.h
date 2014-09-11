/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CURVEPARAM_H_
#define CURVEPARAM_H_

#include <server/common/MDArray.h>
#include <armadillo>

namespace sail {

MDArray2d parameterizeCurve(int vertexCount, Arrayi ctrlInds, int regDeg = 2, bool open = true);

} /* namespace mmm */

#endif /* CURVEPARAM_H_ */
