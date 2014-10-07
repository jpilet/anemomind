/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef ENDIANIOSD_H_
#define ENDIANIOSD_H_

#include "Endian.h"
#include "../FixedPoint/FixedPoint.h"

template <typename T>
T readInteger(File *file) {
  T x;
  char *dst = (char *)(&x);
  for (int i = 0; i < sizeof(T); i++) {
    dst[i] = file->read();
  }
  return convertNativeAndBigEndian(x);
}

template <typename T>
void writeInteger(T x, File *file) {
  T big = convertNativeAndBigEndian(x);
  char *src = (char *)(&big);
  for (int i = 0; i < sizeof(T); i++) {
    file->write(src[i]);
  }
}

template <typename StoreType, typename LongType, int Shift>
void readFixedPoint(FixedPoint<StoreType, LongType, Shift> *dst, File *file) {
  *dst = FixedPoint<StoreType, LongType, Shift>::make(readInteger<StoreType>(file));
}

template <typename StoreType, typename LongType, int Shift>
void writeFixedPoint(FixedPoint<StoreType, LongType, Shift> value, File *file) {
  writeInteger(value.rawFixedPoint(), file);
}




#endif /* ENDIANIOSD_H_ */
