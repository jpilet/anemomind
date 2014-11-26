/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef R2IMAGE_H_
#define R2IMAGE_H_

#include <memory>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {

// Base class for multichannel images defined on R^2 (continuous domain)
template <int Channels>
class R2Image {
 public:
  typedef R2Image<Channels> ThisType;
  typedef std::shared_ptr<ThisType> Ptr;
  typedef Vectorize<double, Channels> Vec;

  virtual Vec operator() (double x, double y) const = 0;

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
