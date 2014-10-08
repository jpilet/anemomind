/*
 *  Created on: 2014-10-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef ENDIAN_H_
#define ENDIAN_H_

inline bool isLittleEndian() {
  unsigned int i = 255;
  return *((unsigned char *)(&i)) == 255;
}

template <typename T>
T flipEndian(const T &src) {
  T dst = 0;
  unsigned char *s = (unsigned char *)(&src);
  unsigned char *d = (unsigned char *)(&dst);
  for (int i = 0; i < sizeof(T); i++) {
    d[sizeof(T) - 1 - i] = s[i];
  }
  return dst;
}

// Reverses byte order of x if this machine has little endian.
template <typename T>
T convertNativeAndBigEndian(const T &x) {
  static bool ile = isLittleEndian();
  if (ile) {
    return flipEndian(x);
  }
  return x;
}



#endif /* ENDIAN_H_ */
