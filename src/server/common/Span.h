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
#include <type_traits>

namespace sail {

template <typename T>
class Span {
 public:
  typedef Span<T> ThisType;

  // This constructor does not initialize _minv and _maxv
  // but since they are of type T, it is hard to do this generically.
  Span() : _initialized(false) {}

  bool initialized() const {
    return _initialized;
  }

  static ThisType centeredAt(T x, T marg) {
    ThisType dst;
    dst.extend(x - marg);
    dst.extend(x + marg);
    return dst;
  }

  static ThisType centeredAt0(T marg) {
    ThisType dst;
    dst.extend(-marg);
    dst.extend(marg);
    return dst;
  }

  Span(T value) : _minv(value), _maxv(value), _initialized(true) {}
  Span(Array<T> arr) {
    _initialized = false;
    for (auto e: arr) {
      extend(e);
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

  bool operator== (const ThisType &other) const {
    return _minv == other._minv && _maxv == other._maxv && _initialized == other._initialized;
  }

  ThisType getWider(T marg) const {
    ThisType dst = *this;
    dst.extend(_minv - marg);
    dst.extend(_maxv + marg);
    return dst;
  }

  T width() const {
    assert(initialized());
    return _maxv - _minv;
  }

  // So that this object can be treated like a sail::Array or std::vector,
  // that also have size() methods.
  T size() const {
    static_assert(std::is_integral<T>::value, "Only integers");
    return width();
  }

  T middle() const {
    return (_minv + _maxv)/2;
  }

  class Iterator {
   public:
    Iterator(T index) : _index(index) {}

    T operator*() const {
      return _index;
    }

    void operator++() {
      _index++;
    }

    bool operator== (Iterator other) const {
      return _index == other._index;
    }

    bool operator!= (Iterator other) const {
      return !((*this) == other);
    }
   private:
    T _index;
  };

  Iterator begin() const {
    assert(_initialized);
    return Iterator(_minv);
  }

  Iterator end() const {
    assert(_initialized);
    return Iterator(_maxv);
  }

  T operator[] (T index) const {
    static_assert(std::is_integral<T>::value, "Only integers");
    return _minv + index;
  }

  Span<T> slice(Span<T> s) const {
    static_assert(std::is_integral<T>::value, "Only integers");
    return Span<T>(_minv + s.minv(), _minv + s.maxv());
  }
 private:
  bool _initialized;
  T _minv, _maxv;
};

typedef Span<double> Spand;
typedef Span<int> Spani;

std::ostream &operator<<(std::ostream &s, const Span<int> &x);
std::ostream &operator<<(std::ostream &s, const Span<double> &x);

template <typename T> class Length;
typedef Span<Length<double> > LengthSpan;

template <typename T> class Duration;
typedef Span<Duration<double> > TimeSpan;

} /* namespace sail */

#endif /* SPAN_H_ */
