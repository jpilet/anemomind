/*
 * BBox.h
 *
 *  Created on: 18 janv. 2014
 *      Author: jonas
 */

#ifndef BBOX_H_
#define BBOX_H_

#include "Span.h"
#include  <iosfwd>

namespace sail {

// A bounding box of N dimensions.
template <int N>
class BBox {
 public:
  typedef BBox<N> ThisType;

  BBox() {}
  BBox(double *vecN) {
    extend(vecN);
  }

  BBox(Span x) {
    static_assert(N == 1, "This constructor is only applicable to 1-D bounding boxes.");
    _span[0] = x;
  }

  void extend(double *vecN) {
    for (int i = 0; i < N; i++) {
      _span[i].extend(vecN[i]);
    }
  }

  void extend(const ThisType &other) {
    for (int i = 0; i < N; i++) {
      _span[i].extend(other._span[i]);
    }
  }

  Span &getSpan(int index) {
    return _span[index];
  }

  virtual ~BBox() {}
 private:
  Span _span[N];
};

typedef BBox<1> BBox1d;
typedef BBox<2> BBox2d;
typedef BBox<3> BBox3d;

std::ostream &operator<< (std::ostream &s, BBox2d box);
std::ostream &operator<< (std::ostream &s, BBox3d box);


} /* namespace sail */

#endif /* BBOX_H_ */
