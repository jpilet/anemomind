/*
 * AbstractArray.h
 *
 *  Created on: May 13, 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_ABSTRACTARRAY_H_
#define SERVER_COMMON_ABSTRACTARRAY_H_

#include <iterator>

namespace sail {

template <typename T>
class AbstractArray;

template <typename T>
class AbstractArrayIterator;

template <typename T>
class AbstractArray {
public:
  virtual T operator[] (int i) const = 0;
  virtual int size() const = 0;
  bool empty() const {
    return size() <= 0;
  }

  AbstractArrayIterator<T> begin() const;
  AbstractArrayIterator<T> end() const;

  virtual ~AbstractArray() {}
};


// http://stackoverflow.com/questions/8054273/how-to-implement
//  -an-stl-style-iterator-and-avoid-common-pitfalls
template <typename T>
class AbstractArrayIterator : public std::iterator<std::random_access_iterator_tag, T, int> {
public:
  typedef AbstractArrayIterator<T> ThisType;
  typedef T value_type;

  AbstractArrayIterator(const AbstractArray<T> *dst, int i)
    : _dst(dst), _index(i) {}

  T operator*() const {
    return (*_dst)[_index];
  }

  bool operator==(const ThisType &other) const {
    return _index == other._index;
  }

  bool operator!=(const ThisType &other) const {
    return _index != other._index;
  }

  ThisType operator-(int i) const {
    return ThisType(_dst, _index - i);
  }

  ThisType operator+(int i) const {
    return ThisType(_dst, _index + i);
  }

  int operator-(const ThisType &other) const {
    return _index - other._index;
  }

  ThisType &operator++() {
    _index++;
    return *this;
  }

  ThisType &operator--() {
    _index--;
    return *this;
  }

  ThisType &operator+=(int i) {
    _index += i;
    return *this;
  }
private:
  const AbstractArray<T> *_dst;
  int _index;
};

template <typename T>
AbstractArrayIterator<T> AbstractArray<T>::begin() const {
  return AbstractArrayIterator<T>(this, 0);
}

template <typename T>
AbstractArrayIterator<T> AbstractArray<T>::end() const {
  return AbstractArrayIterator<T>(this, size());
}


} /* namespace sail */

#endif /* SERVER_COMMON_ABSTRACTARRAY_H_ */
