/*
 * ArrayCopy.h
 *
 *  Created on: 5 May 2017
 *      Author: jonas
 */

#ifndef DEVICE_ANEMOBOX_ARRAYCOPY_H_
#define DEVICE_ANEMOBOX_ARRAYCOPY_H_

#include <boost/noncopyable.hpp>

namespace sail {

// When we need to make local copy and visit every element.
// We provide a size parameter at compile time that will
// determine whether the copy should be on the stack or dynamically
// allocated. I think we could have used alloca
// (http://stackoverflow.com/a/6335048) to always allocate
// on the stack, but it seems a bit risky.
template <typename T, int MaxSize>
class LocalArrayCopy : public boost::noncopyable {
public:
  template <typename It>
  LocalArrayCopy(It from, It to) {
    int n = std::distance(from, to);
    if (n <= MaxSize) {
      _begin = _staticStorage;
      _end = _begin + n;
    } else {
      _dynamicStorage.resize(n);
      _begin = _dynamicStorage.data();
      _end = _begin + n;
    }
    std::copy(from, to, _begin);
  }

  const T* begin() const {return _begin;}
  const T* end() const {return _end;}
private:
  T* _begin = nullptr;
  T* _end = nullptr;
  T _staticStorage[MaxSize];
  std::vector<T> _dynamicStorage;
};

}




#endif /* DEVICE_ANEMOBOX_ARRAYCOPY_H_ */
