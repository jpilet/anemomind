/*
 * AbstractArray.h
 *
 *  Created on: May 13, 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_ABSTRACTARRAY_H_
#define SERVER_COMMON_ABSTRACTARRAY_H_

#include <iterator>
#include <assert.h>
#include <server/common/traits.h>

namespace sail {

template <typename T>
class AbstractArray;

template <typename T>
class AbstractArrayIterator;

template <typename T>
class AbstractArray {
public:
  virtual T operator[] (int i) const = 0;
  virtual size_t size() const = 0;

  // Looks better if we are accessing it through a pointer.
  T get(int i) const {return (*this)[i];}

  bool empty() const {
    return size() <= 0;
  }

  AbstractArrayIterator<T> begin() const;
  AbstractArrayIterator<T> end() const;

  T first() const {
    assert(!empty());
    return (*this)[0];
  }

  T last() const {
    assert(!empty());
    return (*this)[size() - 1];
  }

  virtual ~AbstractArray() {}
};

template <typename T>
class EmptyArray : public AbstractArray<T> {
public:
  size_t size() const override {return 0;}
  T operator[] (int i) const override {return T();}
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

template <typename Indexable, TypeMode mode>
class IndexableWrap : public AbstractArray<typename IndexedType<Indexable>::type> {
public:
  typedef typename IndexedType<Indexable>::type T;

  IndexableWrap(const Indexable &src) : _src(src) {}

  T operator[] (int i) const {
    return _src[i];
  }

  size_t size() const {
    return _src.size();
  }
private:
  typename ModifiedType<Indexable, mode>::type _src;
};

template <TypeMode mode, typename Indexable>
IndexableWrap<Indexable, mode> wrapIndexable(const Indexable &src) {
  return IndexableWrap<Indexable, mode>(src);
}


} /* namespace sail */

#endif /* SERVER_COMMON_ABSTRACTARRAY_H_ */
