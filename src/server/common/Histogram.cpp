/*
 *  Created on: 2014-05-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Histogram.h"

#include <assert.h>
#include <algorithm>
#include <cmath>
#include <server/common/Span.h>
#include <server/common/LineKM.h>

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


void drawPolarBin(double leftRadians, double rightRadians, double radius,
    MDArray2d dst0, int segmentsPerBin, bool nautical) {
  assert(segmentsPerBin + 2 == dst0.rows());
  assert(2 == dst0.cols());
  dst0.sliceRow(0).setAll(0);
  int arcPts = segmentsPerBin + 1;
  MDArray2d dst = dst0.sliceRowsFrom(1);
  LineKM map(0, segmentsPerBin, leftRadians, rightRadians);
  int k = (nautical? 1 : 0);
  for (int i = 0; i < arcPts; i++) {
    double angle = map(i);
    dst(i, 0 + k) = radius*cos(angle);
    dst(i, 1 - k) = radius*sin(angle);
  }
}

}
} /* namespace sail */
