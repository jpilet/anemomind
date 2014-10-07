/*
 *  Created on: 2014-10-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <cstdio>
#include "Endian.h"
#include "../FixedPoint/FixedPoint.h"

#ifndef ENDIANIO_H_
#define ENDIANIO_H_

  /*
   * These routines take big/little endian into account.
   *
   * Using these routines, we will not get into
   * trouble if a file is written on a machine with
   * little endian and read on a machine with big endian.
   */
  template <typename T>
  T freadInteger(FILE *file) {
    T raw;
    fread((void *)(&raw), sizeof(T), 1, file);
    return convertNativeAndBigEndian(raw);
  }

  template <typename T>
  size_t fwriteInteger(T value, FILE *file) {
    T raw = convertNativeAndBigEndian(value);
    return fwrite((void *)(&raw), sizeof(T), 1, file);
  }

  template <typename StoreType, typename LongType, int Shift>
  void freadFixedPoint(FixedPoint<StoreType, LongType, Shift> *dst, FILE *file) {
    *dst = FixedPoint<StoreType, LongType, Shift>::make(freadInteger<StoreType>(file));
  }

  template <typename StoreType, typename LongType, int Shift>
  void fwriteFixedPoint(FixedPoint<StoreType, LongType, Shift> value, FILE *file) {
    fwriteInteger(value.rawFixedPoint(), file);
  }



#endif /* ENDIANIO_H_ */
