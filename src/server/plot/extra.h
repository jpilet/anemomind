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
  /*
   * Switch between lines and points using:
   *  set_style("lines")
   *  set_style("points")
   */

  struct Settings {
    // http://gnuplot.sourceforge.net/docs_4.2/node237.html
    std::string color = "blue";

    // http://stelweb.asu.cas.cz/~nemeth/work/stuff/gnuplot/gnuplot-line-and-point-types-bw.png
    int lineType = 1;
    int pointType = 1;

    // Size
    int lineWidth = 1;
    double pointSize = 1;
  };

  GnuplotExtra();
  //void setHue(double hue);
  void plot(MDArray2d data, const std::string &title = "");
  void plot(int dim, double *from, double *to, const std::string &title = "");
  void show();

  void defineStyle(int index, const Settings &s);

  void setEqualAxes();
 private:
  //std::string _rgbString;
};

void sleepForever();

} /* namespace sail */

#endif /* EXTRA_H_ */
