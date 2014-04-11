/*
 *  Created on: 2014-04-03
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Shared pointer utilities, in addition to those already in the standard library
 */

#ifndef SHARED_H_
#define SHARED_H_

#include <memory>

namespace sail {

/*
 *  WARNING - Improper use of NonDeletingDeleter or makeSharedPtrToStack can
 *  lead to memory corruption.
 *
 *  Do NOT use these unless you are 100% sure of what you are doing !
 */

template <typename T>
class NonDeletingDeleter {
 public:
  void operator() (T *x) {}
};


/*
 * Construct a shared pointer from a reference to an existing
 * object. This pointer will not attempt to deallocate
 * the object once the reference counter reaches zero: This
 * is useful for objects that are allocated on the stack.
 */
template <typename T>
std::shared_ptr<T> makeSharedPtrToStack(T &x) {
  return std::shared_ptr<T>(&x, NonDeletingDeleter<T>());
}

}

#endif /* SHARED_H_ */
