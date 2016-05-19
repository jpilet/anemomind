/*
 * traits.h
 *
 *  Created on: May 13, 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_TRAITS_H_
#define SERVER_COMMON_TRAITS_H_

namespace sail {

template <typename T>
T makeCopy(const T &x) {
  return T(x);
}

template <typename Indexable>
struct IndexedType {
  typedef decltype(makeCopy(std::declval<Indexable>()[0])) type;
};

enum class TypeMode {
  None,
  ConstRef,
  Ref
};

template <typename T, TypeMode mod>
struct ModifiedType {
  typedef T type;
};

template <typename T>
struct ModifiedType<T, TypeMode::ConstRef> {
  typedef const T &type;
};

template <typename T>
struct ModifiedType<T, TypeMode::Ref> {
  typedef T &type;
};


}



#endif /* SERVER_COMMON_TRAITS_H_ */
