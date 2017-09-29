/*
 * traitsTest.cpp
 *
 *  Created on: 7 Sep 2017
 *      Author: jonas
 */

#include <server/common/traits.h>
#include <functional>
#include <sstream>

using namespace sail;

static_assert(
    std::is_same<
      TypeList<int, double>,
        FunctionArgTypes<
          std::function<std::string(int, double)>>>::value, "");

std::string catIntAndDoubleToString(int x, double y) {
  std::stringstream ss;
  ss << x << y;
  return ss.str();
}

static_assert(
    std::is_same<
      TypeList<int, double>,
        FunctionArgTypes<
          decltype(&catIntAndDoubleToString)>>::value, "");

static_assert(
  !std::is_same<
    TypeList<int, int>,
    FunctionArgTypes<
      std::function<std::string(int, double)>>>::value, "");

static_assert(
    std::is_same<
      std::string,
      SecondType<TypeList<int, std::string>>>::value, "");

static_assert(
    3 == TypeListSummary<TypeList<int, int, char>>::size, "");

static_assert(
    std::is_same<TypeMapper<CleanTypeOpKey, int&>::type,
      int>::value, "");

static_assert(
    std::is_same<TypeMapper<CleanTypeOpKey, const int&>::type,
      int>::value, "");

static_assert(
    std::is_same<Cons<int, TypeList<double, char>>::type,
      TypeList<int, double, char>>::value, "");

static_assert(
    std::is_same<TypeMapper<CleanTypeOpKey, const int&>::type,
      int>::value, "");

static_assert(
    std::is_same<
      CleanTypeList<TypeList<const int&,
        double&, const std::string>>,
      TypeList<int, double, std::string>>::value, "");

auto f = [](char x) {return 2.0*int(x);};

static_assert(std::is_same<
    FunctionTraits<decltype(f)>::result_type,
    double>::value, "");

static_assert(std::is_same<
    FunctionTraits<decltype(f)>::arg_types,
    TypeList<char>>::value, "");

int main() {
  return 0;
}


