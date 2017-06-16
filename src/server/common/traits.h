/*
 * traits.h
 *
 *  Created on: May 13, 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_TRAITS_H_
#define SERVER_COMMON_TRAITS_H_

namespace sail {

// To make a reference into a value.
template <typename T>
constexpr T copyOf(T x) {
  return x;
}

template <typename T>
T makeCopy(const T &x) {
  return T(x);
}

template <typename Indexable>
struct IndexedType {
  typedef decltype(makeCopy(std::declval<Indexable>()[0])) type;
};

enum class TypeMode {
  Value,
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

template<typename T>
struct void_ { typedef void type; };

template<typename T, typename = void, typename = void>
struct IsMap {
  static const bool value = false;
};

template<typename T>
struct IsMap <T, typename void_<typename T::key_type>::type,
  typename void_<typename T::mapped_type>::type> {
  static const bool value = true;
  typedef int type;
};

template <typename Iterator, typename = void>
struct IteratorInputType {
  typedef typename std::iterator_traits<Iterator>::value_type type;
};

template <typename Iterator>
struct IteratorInputType<
  Iterator, typename void_<
    typename Iterator::container_type>::type> {
  typedef typename Iterator::container_type C;
  typedef typename C::value_type type;
};

template <typename T, typename = void>
struct IsContainer {
  static const bool value = false;
};

template <typename T>
struct IsContainer<T, typename void_<typename T::value_type>::type> {
  static const bool value = true;
};

template <typename A, typename B>
struct AreSimilar {
  static const bool value =
      std::is_same<
        decltype(copyOf(std::declval<A>())),
        decltype(copyOf(std::declval<B>()))>::value;
};

template <typename T>
struct IsSequenceLike {
  static const bool value = IsContainer<T>::value
      && !IsMap<T>::value
      && !AreSimilar<T, std::string>::value;
};

template <typename T, bool app>
struct KeyType {
  typedef void type;
};

template <typename T>
struct KeyType<T, true> {
  typedef typename T::key_type type;
};

template <typename T>
using KeyTypeOf = typename KeyType<T, IsMap<T>::value>::type;


template <typename T>
struct IsStringMap {
  static const bool value = IsMap<T>::value
      && std::is_same<std::string, KeyTypeOf<T>>::value;
};

}



#endif /* SERVER_COMMON_TRAITS_H_ */
