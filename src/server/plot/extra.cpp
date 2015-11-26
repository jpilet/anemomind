/*
 * extra.cpp
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#include "extra.h"
#include <server/common/string.h>
#include <chrono>
#include <thread>

namespace sail {

//void GnuplotExtra::setHue(double hue)
//{
//	uint8_t RGB[3];
//	hue2RGB(hue, RGB);
//
//}

GnuplotExtra::GnuplotExtra() {
  //_rgbString = "";
}

void GnuplotExtra::plot(MDArray2d data, const std::string &title) {
  int rows = data.rows();
  if (data.cols() == 2) {
    plot_xy(data.sliceCol(0).getStorage().sliceTo(rows),
            data.sliceCol(1).getStorage().sliceTo(rows),
            title);
  } else if (data.cols() == 3) {
    plot_xyz(data.sliceCol(0).getStorage().sliceTo(rows),
             data.sliceCol(1).getStorage().sliceTo(rows),
             data.sliceCol(2).getStorage().sliceTo(rows),
             title);
  } else {
    cerr << "BAD NUMBER OF COLUMNS" << endl;
    throw std::exception();
  }
}

void GnuplotExtra::plot(int dim, double *from, double *to, const std::string &title) {
  double X[2] = {from[0], to[0]};
  double Y[2] = {from[1], to[1]};
  if (dim == 2) {
    plot_xy(Arrayd(2, X), Arrayd(2, Y), title);
  } else {
    double Z[2] = {from[2], to[2]};
    plot_xyz(Arrayd(2, X), Arrayd(2, Y), Arrayd(2, Z), title);
  }
}

void GnuplotExtra::show() {
  showonscreen();
}

void sleepForever() {
  std::this_thread::sleep_for(std::chrono::seconds(30000));
}

void GnuplotExtra::setLineStyle(int index, std::string colorCode, int lineWidth) {
  std::stringstream ss;
  ss << "set style line " << index << " lt rgb \"" << colorCode << "\" lw " << lineWidth;
  cmd(ss.str());
}

void GnuplotExtra::setLineStyle(int index, const Settings &s) {
  std::stringstream ss;
  ss << "set style line " << index << " lt \"" << s.color
      << "\" lw " << s.lineWidth << " pt " << s.pointType << " ps " << s.pointSize;
  cmd(ss.str());
}

void GnuplotExtra::setEqualAxes() {
  cmd("set size ratio -1");
}





} /* namespace sail */
