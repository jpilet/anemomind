/*
 *  Created on: 2014-04-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef ENV_H_
#define ENV_H_

namespace sail {

class Env {
 public:

  // X_DIR below maps to a corresponding CMAKE_X_DIR variable.
  static const char *SOURCE_DIR;
  static const char *BINARY_DIR;
 };

} /* namespace sail */

#endif /* ENV_H_ */
