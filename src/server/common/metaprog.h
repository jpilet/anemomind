/*
 * metaprog.h
 *
 *  Created on: 28 Jul 2017
 *      Author: jonas
 */

#ifndef SERVER_COMMON_METAPROG_H_
#define SERVER_COMMON_METAPROG_H_

#include <utility>

namespace sail {
namespace meta {

// A list of types
template <typename ... T>
struct List {};

// Adding an item at the beginning of the list
template <typename X, typename L>
struct Cons {};

template <typename X, typename ... Y>
struct Cons<X, List<Y...>> {
  typedef List<X, Y...> type;
};

template <typename T, typename List>
struct InsertUnique {};

template <typename T>
struct InsertUnique<T, List<>> {
  typedef List<T> type;
};

template <typename T, typename ... Y>
struct InsertUnique<T, List<T, Y...>> {
  typedef List<T, Y...> type;
};

template <typename X, typename Y, typename ... Z>
struct InsertUnique<X, List<Y, Z...>> {
  typedef typename Cons<Y,
      typename InsertUnique<X, List<Z...>>::type>::type type;
};

template <typename L>
struct MakeSet {};

template <>
struct MakeSet<List<>> {
  typedef List<> type;
};

template <typename X, typename ... Y>
struct MakeSet<List<X, Y...>> {
  typedef typename InsertUnique<X,
      typename MakeSet<List<Y...>>::type>::type type;
};

template <typename T>
class The_type_is;
#define DISP_TYPE(TYPE) sail::meta::The_type_is<TYPE> variable_of_type_to_display


/*
static_assert(std::is_same<List<int>,
    typename MakeSet<List<int, int, int, int>>::type>::value, "");
static_assert(
    std::is_same<
      typename MakeSet<List<double, int>>::type,
      typename MakeSet<List<double, int, double, int, int, int>>::type>::value, "");
static_assert(std::is_same<List<int>, typename InsertUnique<int, List<>>::type>::value, "");
static_assert(std::is_same<
    typename InsertUnique<int, List<int, double>>::type,
    List<int, double>>::value, "");

static_assert(std::is_same<List<double, float, bool>,
    typename InsertUnique<bool, List<double, float>>::type>::value, "");
static_assert(std::is_same<List<double, float>,
    typename InsertUnique<float, List<double, float>>::type>::value, "");
*/

}
}



#endif /* SERVER_COMMON_METAPROG_H_ */
