/*
 *  Created on: 2014-04-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Use ArrayBuilder to add elements to an array when the final number of
 *  elements is not known.
 */

#ifndef ARRAYBUILDER_H_
#define ARRAYBUILDER_H_

#include <server/common/Array.h>

namespace sail {

template <typename T>
class ArrayBuilder {
 public:
  ArrayBuilder(int expectedMaxCount = 1) :
    _counter(0), _dst(Array<T>(expectedMaxCount)) {
    assert(expectedMaxCount >= 1); // If not, newSize = 2*old.size() = 0, that is, the array will not be able to grow.
  }

  void add(const T &x) {
    if (full()) {
      reallocate();
    }
    _dst[_counter] = x;
    _counter++;
  }

  Array<T> get() {return _dst.sliceTo(_counter);}
 private:
  bool full() {return _counter == _dst.size();}
  void reallocate() {
    Array<T> old = _dst;
    assert(old.size() > 0);
    int newSize = 2*old.size();
    _dst = Array<T>(newSize);
    old.copyToSafe(_dst.sliceTo(_counter));
  }
  int _counter;
  Array<T> _dst;
};

}

#endif /* ARRAYBUILDER_H_ */
