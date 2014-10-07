/*
 *  Created on: 2014-10-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef ENDIANIOSTREAM_H_
#define ENDIANIOSTREAM_H_

#include "Endian.h"
#include "../FixedPoint/FixedPoint.h"

 /*
   * These routines take big/little endian into account.
   *
   * Using these routines, we will not get into
   * trouble if a file is written on a machine with
   * little endian and read on a machine with big endian.
   *
   * To be used with <cstdio> functions
   */
template <typename T>
void writeBinaryData(const T &src, std::ostream *dst) {
  dst->write((char *)(&src), sizeof(T));
}


template <typename T>
bool readBinaryData(std::istream *src, T *dst) {
  src->read((char *)(&dst), sizeof(T));
  return true;
}

template <typename T>
void writeBinaryInteger(const T &src, std::ostream *dst) {
  writeBinaryData(convertNativeAndBigEndian(src), dst);
}

template <typename T>
bool readBinaryInteger(std::istream *src, T *dst) {
  T temp = 0;
  bool v = readBinaryData(src, temp);
  dst = convertNativeAndBigEndian(temp);
  return v;
}

template <typename StoreType, typename LongType, int Shift>
void writeBinaryFixedPoint(FixedPoint<StoreType, LongType, Shift> value, std::ostream *dst) {
  writeBinaryInteger(value.rawFixedPoint(), dst);
}

template <typename StoreType, typename LongType, int Shift>
bool readBinaryFixedPoint(std::istream *src, FixedPoint<StoreType, LongType, Shift> *dst) {
  StoreType x;
  if (readBinaryInteger<StoreType>(src, &x)) {
    *dst = FixedPoint<StoreType, LongType, Shift>::make(x);
    return true;
  }
  return false;
}





#endif /* ENDIANIOSTREAM_H_ */
