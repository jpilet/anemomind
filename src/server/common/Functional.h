/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 *
 *  Functions that operate on collection type objects.
 *  Objects that are indexable with the [] operator and has a .size() method,
 *  such as std::vector, sail::Array, sail::Spani, etc.
 */

#ifndef SERVER_COMMON_FUNCTIONAL_H_
#define SERVER_COMMON_FUNCTIONAL_H_

#include <server/common/Array.h>
#include <server/common/ArrayBuilder.h>
#include <utility>
#include <server/common/logging.h>
#include <cassert>
#include <server/common/traits.h>

namespace sail {

template <typename F>
struct ComplementFunction {
  F f;
  template <typename ... T>
  bool operator()(T ... x) {
    return !f(x...);
  }
};

// Constructs a functor that returns
// a true value iff f returns a falsy value,
// otherwise it returns false.
template <typename F>
ComplementFunction<F> complementFunction(F f) {
  return ComplementFunction<F>{f};
}

}

#undef ADD_METHODS_FOR_MAPPED
#endif /* SERVER_COMMON_FUNCTIONAL_H_ */
