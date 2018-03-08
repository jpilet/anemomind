/*
 * VariantIterator.hpp
 *
 *  Created on: 8 Mar 2018
 *      Author: jonas
 */

#ifndef SERVER_COMMON_VARIANTITERATOR_H_
#define SERVER_COMMON_VARIANTITERATOR_H_

#include <boost/variant.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <server/common/IndexedValue.h>

namespace sail {

template <int Index, typename ... T>
struct VariantIteratorWrapper {
  typedef boost::variant<T...> Variant;

  struct F {
    template <typename T>
    IndexedValue<Variant> operator()(T x) const {
      return IndexedValue(Index, x);
    }
  };

  template <typename Iterator>
  boost::transform_iterator<F, Iterator> wrap(Iterator iter) const {
    F f;
    return boost::transform_iterator<F, Iterator>(f(), iter);
  }
};

}



#endif /* SERVER_COMMON_VARIANTITERATOR_H_ */
