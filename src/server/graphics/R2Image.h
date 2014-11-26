/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef R2IMAGE_H_
#define R2IMAGE_H_

namespace sail {

// Base class for multichannel images defined on R^2 (continuous domain)
template <int Channels>
class R2Image {
 public:
  typedef std::shared_ptr<R2Image> Ptr;
  typedef Vectorize<double, 3> Vec;

  virtual Vec operator() (double x, double y) const = 0;

  virtual ~R2Image() {}
};

typedef R2Image<3> R2ImageRGB;

template <int Channels>
class R2ImageTranslate : public R2Image<Channels> {
 public:
  R2ImageTranslate(Ptr dst, double xt, double yt) :
    _dst(dst), _xt(xt), _yt(yt) {}

  Vec operator() (double x, double y) const {
    return (*_dst)(x - _xt, y - _yt);
  }
 private:
  R2Image<Channels>::Ptr _dst;
  double _xt, _yt;
};


}


#endif /* R2IMAGE_H_ */
