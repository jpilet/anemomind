/*
 *  Created on: 2014-04-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef ENDIANESS_H_
#define ENDIANESS_H_

namespace sail {

// http://stackoverflow.com/a/1001373
inline int isBigEndian() {
  union {
      uint32_t i;
      char c[4];
  } bint = {0x01020304};
  return bint.c[0] == 1;
}

}





#endif /* ENDIANESS_H_ */
