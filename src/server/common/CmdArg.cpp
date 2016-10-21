/*
 * CmdArg.cpp
 *
 *  Created on: 20 Oct 2016
 *      Author: jonas
 */

#include <server/common/CmdArg.h>

namespace sail {

template <typename T>
Arg<T> &Arg<T>::describe(const std::string &d) {
  _desc = d;
}

template class Arg<int>;
template class Arg<double>;
template class Arg<std::string>;
template class Arg<bool>;
template class Arg<char>;

CmdArg::CmdArg(const std::string &desc) : _desc(desc) {}

}

