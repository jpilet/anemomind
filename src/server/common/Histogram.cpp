/*
 *  Created on: 2014-05-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Histogram.h"

#include <assert.h>
#include <algorithm>
#include <cmath>
#include <server/common/Span.h>

namespace sail {
namespace HistogramInternal {

void drawBin(double left, double right, double height, MDArray2d dst) {
    assert(dst.rows() == 3);
    assert(dst.cols() == 2);

    dst(0, 0) = left;
    dst(0, 1) = 0;

    dst(1, 0) = left;
    dst(1, 1) = height;

    dst(2, 0) = right;
    dst(2, 1) = height;
  }

}
} /* namespace sail */
