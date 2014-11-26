/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/graphics/PPM.h>
#include <iostream>

using namespace sail;

namespace {
  RGBByteImage makeImage() {
    RGBByteImage im(300, 200);
    double xc = 150;
    double yc = 100;
    double r = 60;
    double r2 = sqr(r);
    Vectorize<unsigned char, 3> blue{0, 0, 255};
    Vectorize<unsigned char, 3> red{255, 0, 0};
    for (int y = 0; y < 200; y++) {
      for (int x = 0; x < 300; x++) {
        im(x, y) = (sqr(x - xc) + sqr(y - yc) < r2? blue : red);
      }
    }
    return im;
  }
}

int main(int argc, const char **argv) {
  if (argc <= 1) {
    std::cout << "Please provide an output filename" << std::endl;
  } else {
    writePPM("testimage.ppm"/*argv[1]*/, makeImage());
    std::cout << "Wrote image with a blue circle on a red background" << std::endl;
    std::cout << "Width should be 300, height 200" << std::endl;
  }
  return 0;
}


