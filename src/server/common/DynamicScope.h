/*
 * DynamicScope.hpp
 *
 *  Created on: 12 Oct 2017
 *      Author: jonas
 */

#ifndef SERVER_COMMON_DYNAMICSCOPE_H_
#define SERVER_COMMON_DYNAMICSCOPE_H_

#include <boost/core/noncopyable.hpp>

namespace sail {

// Use to temporarily set a variable to something in this scope.
template <typename T>
class Bind : public boost::noncopyable {
public:
  Bind(T* target, const T& newValue)
    : _target(target), _value(newValue), _old(*target) {
    *_target = _value;
  }

  ~Bind() {
    *_target = _old;
  }
private:
  T _old;
  T _value;
  T* _target;
};

}




#endif /* SERVER_COMMON_DYNAMICSCOPE_H_ */
