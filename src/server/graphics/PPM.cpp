/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/graphics/PPM.h>
#include <fstream>

namespace sail {

void writePPM(std::string filename, RGBByteImage image) {
  std::ofstream file(filename);
  file << "P3\n";
  int w = image.width();
  int h = image.height();
  file << w << " " << h << "\n" << 255 << "\n";
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      auto &pixel = image(x, y);
      for (int c = 0; c < 3; c++) {
        file << pixel[c] << " ";
      }
      file << " ";
    }
    file << "\n";
  }
}

}
