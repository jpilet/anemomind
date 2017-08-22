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
template <typename T, int N>
class BBox {
 public:
  typedef BBox<T, N> ThisType;

  BBox() {}
  BBox(T *vecN) {
    extend(vecN);
  }

  BBox(Span<T> *spans) {
    for (int i = 0; i < N; i++) {
      _span[i] = spans[i];
    }
  }

  BBox(Span<T> x) {
    static_assert(N == 1, "This constructor is only applicable to 1-D bounding boxes.");
    _span[0] = x;
  }

  void extend(T *vecN) {
    for (int i = 0; i < N; i++) {
      _span[i].extend(vecN[i]);
    }
  }

  void extend(const std::initializer_list<double> &list) {
    assert(N == list.size());
    T v[N];
    int counter = 0;
    for (auto x: list) {
      v[counter++] = x;
    }
    assert(counter == N);
    extend(v);
  }

  void extend(const ThisType &other) {
    for (int i = 0; i < N; i++) {
      _span[i].extend(other._span[i]);
    }
  }

  const Span<T> &getSpan(int index) const {
    return _span[index];
  }

  ~BBox() {}

  bool operator==(const ThisType &other) const {
    for (int i = 0; i < N; i++) {
      if (!(_span[i] == other._span[i])) {
        return false;
      }
    }
    return true;
  }
 private:
  Span<T> _span[N];
};

typedef BBox<double, 1> BBox1d;
typedef BBox<double, 2> BBox2d;
typedef BBox<double, 3> BBox3d;

std::ostream &operator<< (std::ostream &s, BBox2d box);
std::ostream &operator<< (std::ostream &s, BBox3d box);


} /* namespace sail */

#endif /* BBOX_H_ */
