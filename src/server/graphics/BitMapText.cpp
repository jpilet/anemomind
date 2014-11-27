/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "BitMapText.h"
#include <server/common/string.h>
#include <iostream>

namespace sail {

namespace {
  bool rowHasData(BoolMapImage im, int y) {
    int w = im.width();
    for (int x = 0; x < w; x++) {
      if (im(x, y)[0]) {
        return true;
      }
    }
    return false;
  }

  bool colHasData(BoolMapImage im, int x) {
    int h = im.height();
    for (int y = 0; y < h; y++) {
      if (im(x, y)[0]) {
        return true;
      }
    }
    return false;
  }

  int leftmost(BoolMapImage im) {
    for (int i = 0; i < im.width(); i++) {
      if (colHasData(im, i)) {
        return i;
      }
    }
    return 0;
  }

  int rightmost(BoolMapImage im) {
    for (int i = im.width(); i > 0; i--) {
      if (colHasData(im, i-1)) {
        return i;
      }
    }
    return im.width();
  }

  int topmost(BoolMapImage im) {
    for (int i = 0; i < im.height(); i++) {
      if (rowHasData(im, i)) {
        return i;
      }
    }
    return 0;
  }

  int bottommost(BoolMapImage im) {
    for (int i = im.height(); i > 0; i--) {
      if (rowHasData(im, i-1)) {
        return i;
      }
    }
    return im.height();
  }
}

BitMapText::BitMapText(BoolMapImage im) :
    _im(im) {
    {
      auto s = im.getStorage();
      int n = s.size();
      int counter = 0;
      for (int i = 0; i < n; i++) {
        if (s[i][0]) {
          counter++;
        }
      }
      std::cout << EXPR_AND_VAL_AS_STRING(counter) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(n) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(double(counter)/n) << std::endl;
    }

  _ymap = LineKM(0, 1, topmost(im), bottommost(im));
  _xmap = LineKM(_ymap.getK(), leftmost(im));
  _width = _xmap.inv(rightmost(im));

  std::cout << EXPR_AND_VAL_AS_STRING(_ymap(0)) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(_ymap(1)) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(_xmap(0)) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(_xmap(_width)) << std::endl;
}

bool BitMapText::operator() (double x, double y) const {
  int ix = int(floor(_xmap(x)));
  int iy = int(floor(_ymap(y)));
  if (_im.valid(ix, iy)) {
    return _im(ix, iy)[0];
  }
  return false;
}


} /* namespace mmm */
