/*
 * traits.h
 *
 *  Created on: May 13, 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_TRAITS_H_
#define SERVER_COMMON_TRAITS_H_

#include <functional>

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


template <typename ... T> struct TypeList {};

template <typename T>
struct FunctionTraits {};

template <typename Y, typename ... T>
struct FunctionTraits<std::function<Y(T...)>> {
  typedef Y result_type;
  typedef TypeList<T...> arg_types;
};

template <typename F>
using FunctionResultType = typename FunctionTraits<F>::result_type;

template <typename F>
using FunctionArgTypes = typename FunctionTraits<F>::arg_types;

template <typename Y, typename ... T>
struct FunctionTraits<Y(*)(T...)> {
  typedef Y result_type;
  typedef TypeList<T...> arg_types;
};

template <typename T> struct TypeListOps;
template <typename X, typename ... Y> struct TypeListOps<TypeList<X, Y...>> {
  typedef X first;
  typedef TypeList<Y...> rest;
};

template <typename T>
using FirstType = typename TypeListOps<T>::first;

template <typename T>
using RestTypes = typename TypeListOps<T>::rest;

template <typename T>
using SecondType = FirstType<RestTypes<T>>;

template <typename T> struct TypeListSummary {};

template <typename X, typename ... Y>
struct TypeListSummary<TypeList<X, Y...>> {
  static const int size = 1 + TypeListSummary<TypeList<Y...>>::size;
};

template <>
struct TypeListSummary<TypeList<>> {
  static const int size = 0;
};

}



#endif /* SERVER_COMMON_TRAITS_H_ */
