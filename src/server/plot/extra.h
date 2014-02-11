/*
 * extra.h
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#ifndef EXTRA_H_
#define EXTRA_H_

#include <string>
#include "../common/MDArray.h"
#include "gnuplot_i.hpp"

namespace sail {

class GnuplotExtra : public Gnuplot {
 public:
  GnuplotExtra();
  //void setHue(double hue);
  void plot(MDArray2d data);
  void show();
 private:
  //std::string _rgbString;
};

void sleepForever();

} /* namespace sail */

#endif /* EXTRA_H_ */
