/*
 * Span.h
 *
 *  Created on: 18 janv. 2014
 *      Author: jonas
 */

#ifndef SPAN_H_
#define SPAN_H_

#include <iosfwd> // std::ostream
#include <algorithm> // std::max
#include <assert.h>
#include <server/common/Array.h>

namespace sail {

template <typename T>
class Span {
 public:
  // This constructor does not initialize _minv and _maxv
  // but since they are of type T, it is hard to do this generically.
  Span() : _initialized(false) {}

  bool initialized() const {
    return _initialized;
  }

  Span(T value) : _minv(value), _maxv(value), _initialized(true) {}
  Span(Array<T> arr) {
    if (arr.empty()) {
      _initialized = false;
    } else {
      _initialized = false;
      int count = arr.size();
      _minv = arr[0];
      _maxv = _minv;
      for (int i = 1; i < count; i++) {
        extend(arr[i]);
      }
      _initialized = true;
    }
  }


  T minv() const {
    assert(_initialized);
    return _minv;
  }
  T maxv() const {
    assert(_initialized);
    return _maxv;
  }


  bool contains(T value) const {
    assert(_initialized);
    return _minv <= value && value < _maxv;
  }

  virtual ~Span() {}

  void extend(const Span &other) {
    if (other.initialized()) {
      if (initialized()) {
        _minv = std::min(_minv, other._minv);
        _maxv = std::max(_maxv, other._maxv);
      } else {
        _minv = other._minv;
        _maxv = other._maxv;
        _initialized = true;
      }
    }
  }

  Span(T a, T b) : _initialized(true) {
    if (a <= b) {
      _minv = a;
      _maxv = b;
    } else {
      _minv = b;
      _maxv = a;
    }
  }

  void extend(T value) {
    if (initialized()) {
      _minv = std::min(_minv, value);
      _maxv = std::max(_maxv, value);
    } else {
      _minv = value;
      _maxv = value;
      _initialized = true;
    }
  }

  bool intersects(const Span<T> &other) const {
    assert(initialized());
    assert(other.initialized());
    return contains(other._minv) || contains(other._maxv);
  }
 private:
  bool _initialized;
  T _minv, _maxv;
};

typedef Span<double> Spand;

std::ostream &operator<<(std::ostream &s, const Spand &x);

} /* namespace sail */

#endif /* SPAN_H_ */
