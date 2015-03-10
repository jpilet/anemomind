/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/FlowErrors.h>

namespace sail {


std::ostream &operator<< (std::ostream &s, const FlowErrors &e) {
  s << "FlowError( norm: " << e.norm() << " angle: " << e.angle() << " magnitude: " << e.magnitude() << ")";
  return s;
}


} /* namespace mmm */
