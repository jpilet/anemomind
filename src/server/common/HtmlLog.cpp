/*
 * HtmlLog.cpp
 *
 *  Created on: 16 Sep 2016
 *      Author: jonas
 */

#include "HtmlLog.h"
#include <sstream>

namespace sail {

AttribValue::AttribValue(double v) {
  std::stringstream ss;
  ss << v;
  _value = ss.str();
}

AttribValue::AttribValue(int v) {
  std::stringstream ss;
  ss << v;
  _value = ss.str();
}


} /* namespace sail */
