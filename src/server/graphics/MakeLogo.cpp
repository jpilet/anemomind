/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/graphics/AnemoFont.h>
#include <server/graphics/PPM.h>
#include <iostream>

using namespace sail;

int main(int argc, const char **argv) {
  std::string s = "anemomind";
  anemofont::Renderer renderer;
  for (int i = 0; i < s.size(); i++) {
    renderer.write(s[i]);
  }
  writePPM("/home/jonas/Desktop/logo.ppm", renderer.render(1000));
  std::cout << "Rendered logo" << std::endl;
  return 0;
}


