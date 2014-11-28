/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/graphics/AnemoFont.h>
#include <server/graphics/PPM.h>
#include <server/graphics/TextDistort.h>
#include <iostream>
#include <server/graphics/PBM.h>
#include <server/graphics/BitMapText.h>
#include <server/graphics/DualColorText.h>
#include <server/graphics/Logo.h>

using namespace sail;

int main(int argc, const char **argv) {
  if (true) {

    Logo logo(makeLogoSettings6());
    writePPM("/home/jonas/Desktop/logo.ppm", logo.render(1000));


  } else {
    BoolMapImage bm = readPBM("/home/jonas/Documents/anemomind.pbm");
    BitMapText bmt(bm);
    std::cout << EXPR_AND_VAL_AS_STRING(bmt.width()) << std::endl;

    std::string s = "anemomind";
    anemofont::Renderer *renderer = new anemofont::Renderer();
    for (int i = 0; i < s.size(); i++) {
      renderer->write(s[i]);
    }

    //double slant = 0.3;
    //double bend = 0; //0.5;

    double slant = 0.2;
    double bend = 0.5; //0.5;


    double y[3] = {slant, 0.5*(1 + bend)*slant, 0};
    //:212, G:0, B:87---
    double f = 1.0/255;
    R2ImageRGB::Vec bg{1, 1, 1};
    R2ImageRGB::Vec fg1{0, 0, 0};
    R2ImageRGB::Vec fg2{f*212, 0, f*87};
    R2ImageRGB::Ptr dc(new DualColorText(bmt, bg, fg1, fg2, 0.58));

    std::cout << EXPR_AND_VAL_AS_STRING(dc->width()) << std::endl;
    std::cout << EXPR_AND_VAL_AS_STRING(dc->height()) << std::endl;

    TextDistort::Ptr td(new TextDistort(y, dc));
    R2ImageRemapXY<3> remap(td, LineKM(0.5, 0.0), LineKM::identity());
    writePPM("/home/jonas/Desktop/logo.ppm", remap.render(1000));
    //writePPM("/home/jonas/Desktop/logo.ppm", dc->render(1000));
    std::cout << "Rendered logo" << std::endl;
  }
  return 0;
}


