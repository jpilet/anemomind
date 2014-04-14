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
#include <vector>

namespace sail {

template <typename T>
class ArrayBuilder {
 public:
  ArrayBuilder(int expectedMaxCount = 1) {
    _data.reserve(expectedMaxCount);
  }

  void add(const T &x) {
    _data.push_back(x);
  }

  Array<T> get() {return Array<T>::referToVector(_data).dup();}
 private:
  std::vector<T> _data;
};

}

#endif /* ARRAYBUILDER_H_ */
