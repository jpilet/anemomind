/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef BITMAPTEXT_H_
#define BITMAPTEXT_H_

#include <server/graphics/R2Image.h>

namespace sail {

class BitMapText : public R2Image1 {
 public:
  BitMapText(BoolMapImage im);
  Vec operator() (double x, double y) const;

  double height() const {
    return 1.0;
  }

  double width() const {
    return _width;
  }
 private:
  BoolMapImage _im;
  LineKM _xmap, _ymap;
  double _width;
};

}

#endif /* BITMAPTEXT_H_ */
