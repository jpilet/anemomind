/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/graphics/AnemoFont.h>
#include <server/graphics/PPM.h>
#include <server/graphics/TextDistort.h>
#include <iostream>

using namespace sail;

int main(int argc, const char **argv) {
  std::string s = "anemomind";
  anemofont::Renderer *renderer = new anemofont::Renderer();
  for (int i = 0; i < s.size(); i++) {
    renderer->write(s[i]);
  }
  double y[3] = {0.2, 0.2, 0};
  TextDistort td(y, R2ImageRGB::Ptr(renderer));
  writePPM("/home/jonas/Desktop/logo.ppm", td.render(1000));
  std::cout << "Rendered logo" << std::endl;
  return 0;
}


