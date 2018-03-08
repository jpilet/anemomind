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
#include <server/common/traits.h>

namespace sail {

template <int Index, typename ... T>
struct VariantIteratorWrapper {
  typedef boost::variant<T...> Variant;
  static constexpr int N = sizeof...(T);
  typedef typename Nth<Index, T...>::type TypeAtIndex;

  typedef VariantIteratorWrapper<Index, T...> ThisType;
  typedef VariantIteratorWrapper<Index+1, T...> NextType;

  NextType next() const {
    return NextType();
  }

  /*struct F {
    TimedValue<IndexedValue<Variant>> operator()(
        TimedValue<TypeAtIndex> x) const {
      return TimedValue<IndexedValue<Variant>>{
        x.time, {Index, x.value}
      };
    }
  };*/

  /*template <typename X>
  TimedValue<IndexedValue<Variant>> operator()(
      const X& x) const {
    return TimedValue<IndexedValue<Variant>>{
      x.time, {Index, x.value}
    };
  }*/

  TimedValue<IndexedValue<Variant>> operator()(
      const TimedValue<TypeAtIndex>& x) const {
    return TimedValue<IndexedValue<Variant>>{
      x.time, {Index, x.value}
    };
  }

  // Returns a transform iterator that will
  // wrap the values of iter in a boost variant and
  // put them in IndexedValues
  template <typename Iterator>
  boost::transform_iterator<ThisType, Iterator> wrap(
      Iterator iter) const {
    return boost::transform_iterator<ThisType, Iterator>(
        iter, ThisType());
  }



  TimedValue<TypeAtIndex> get(
      const std::array<TimedValue<Variant>, N>& v) const {
    return boost::get<TypeAtIndex>(v[Index]);
  }
};

}



#endif /* SERVER_COMMON_VARIANTITERATOR_H_ */
