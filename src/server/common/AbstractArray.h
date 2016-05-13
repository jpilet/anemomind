/*
 * AbstractArray.h
 *
 *  Created on: May 13, 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_ABSTRACTARRAY_H_
#define SERVER_COMMON_ABSTRACTARRAY_H_

namespace sail {

template <typename T>
class AbstractArray {
public:
  virtual T operator[] (int i) const = 0;
  virtual int size() const = 0;
  virtual ~AbstractArray() {}
};

} /* namespace sail */

#endif /* SERVER_COMMON_ABSTRACTARRAY_H_ */
