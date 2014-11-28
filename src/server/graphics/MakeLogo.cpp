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

    const int count = 3;
    Logo::Settings settings[3] = {makeLogoSettings2(), makeLogoSettings7(), makeLogoSettings8()};

    for (int j = 0; j < 2; j++) {
      bool mirror = (j == 0? false : true);
      for (int i = 0; i < 3; i++) {
        Logo::Settings s = settings[i];

        BoolMapImage bm = readPBM("/home/jonas/Images/anemomind/anemomind_neutra.pbm");
        BitMapText bmt(bm);

        Logo::Colors colors;

        double scale = 0.9;
        double margin = 0.18;
        LineKM xmap(scale, margin);
        LineKM ymap(scale, 0.0);

        R2ImageRGB::Ptr dc(new R2ImageRemapXY<3>(
            R2ImageRGB::Ptr(new DualColorText(bmt, colors.bg, colors.max, colors.max, 1.0)),
            xmap, ymap));
        Logo *logo = new Logo(s);
        logo->setSize(1.0, 1.2);
        Combine combine(Logo::Ptr(logo), dc, mirror);
        std::cout << EXPR_AND_VAL_AS_STRING(combine.width()) << std::endl;
        std::cout << EXPR_AND_VAL_AS_STRING(combine.height()) << std::endl;
        writePPM(stringFormat("/home/jonas/Desktop/logo%d_%d.ppm", i, mirror), combine.render(1000));
      }
    }

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


