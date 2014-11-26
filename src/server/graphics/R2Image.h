/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef R2IMAGE_H_
#define R2IMAGE_H_

#include <memory>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/graphics/RasterImage.h>
#include <server/common/LineKM.h>
#include <iostream>
#include <server/common/string.h>

namespace sail {

// Base class for multichannel images defined on R^2 (continuous domain)
template <int Channels>
class R2Image {
 public:
  typedef R2Image<Channels> ThisType;
  typedef std::shared_ptr<ThisType> Ptr;
  typedef Vectorize<double, Channels> Vec;

  virtual Vec operator() (double x, double y) const = 0;

  // Overriding them is optional
  virtual double width() const {return NAN;}
  virtual double height() const {return NAN;}

  bool definedBounds() const {
    return !std::isnan(width()) && !std::isnan(height());
  }

  RasterImage<unsigned char, Channels> render(int dstWidth, int dstHeight = -1) {
    assert(definedBounds());
    assert(height() > 0);
    assert(width() > 0);
    if (dstHeight == -1) {
      dstHeight = int(round((height()/width())*dstWidth));
    }
    assert(dstWidth > 0);
    assert(dstHeight > 0);
    LineKM xmap(0, dstWidth, 0, width());
    LineKM ymap(0, dstHeight, 0, height());
    RasterImage<unsigned char, Channels> dst(dstWidth, dstHeight);
    for (int y = 0; y < dstHeight; y++) {
      for (int x = 0; x < dstWidth; x++) {
        Vectorize<double, Channels> srcpixel = (*this)(xmap(x), ymap(y));
        Vectorize<unsigned char, Channels> &dstpixel = dst(x, y);
        for (int i = 0; i < Channels; i++) {
          dstpixel[i] = (unsigned char)(round(255.0*srcpixel[i]));
        }
      }
    }
    return dst;
  }



  virtual ~R2Image() {}
};

typedef R2Image<3> R2ImageRGB;

template <int Channels>
class R2ImageTranslate : public R2Image<Channels> {
 public:
  typedef typename R2Image<Channels>::Ptr Ptr;
  typedef typename R2Image<Channels>::Vec Vec;

  R2ImageTranslate(Ptr dst,
      double xt, double yt) :
    _dst(dst), _xt(xt), _yt(yt) {}

  Vec operator() (double x, double y) const {
    return (*_dst)(x - _xt, y - _yt);
  }
 private:
  Ptr _dst;
  double _xt, _yt;
};


}


#endif /* R2IMAGE_H_ */
