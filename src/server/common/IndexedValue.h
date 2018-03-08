/*
 * IndexedValue.h
 *
 *  Created on: 8 Mar 2018
 *      Author: jonas
 */

#ifndef SERVER_COMMON_INDEXEDVALUE_H_
#define SERVER_COMMON_INDEXEDVALUE_H_

#include <stdint.h>

namespace sail {

template <typename T>
struct IndexedValue {
  int64_t index = -1;
  T value = T();

  bool defined() const {return index != -1;}
  IndexedValue() {}
  IndexedValue(int64_t i, const T& v) : index(i), value(v) {}
};

}



#endif /* SERVER_COMMON_INDEXEDVALUE_H_ */
