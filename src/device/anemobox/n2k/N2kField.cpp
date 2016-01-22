/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include "N2kField.h"

namespace N2kField {

constexpr int byteLimit = 8;
constexpr int bitLimit = 8*byteLimit;

// Check if the numBits'th most significant bit is set to 1, meaning the number is < 0
// in two's complement representation.
// All bits that are more significant, and not used for representing the number,
// are assumed to be zero, since value originates from the getUnsigned method.
bool isTwosComplementNegative(uint64_t value, int numBits) {
  return (value >> (numBits - 1)) != 0;
}

uint64_t getMaxUnsignedValue(int numBits) {
  return (~static_cast<uint64_t>(0)) >> (bitLimit - numBits);
}

// not needed, but included for the sake of completeness.
uint64_t getMaxSignedValueTwosComplement(int numBits) {
  return getMaxUnsignedValue(numBits) >> 1;
}

uint64_t getMaxSignedValue(int numBits, int64_t offset) {
  if (offset == 0) {
    return getMaxSignedValueTwosComplement(numBits);
  }
  return static_cast<int64_t>(getMaxUnsignedValue(numBits)) + offset;
}


Optional<uint64_t> Stream::getUnsigned(int bits, bool canBeUndefined) {
  if (canRead(bits)) {
    auto x = BitStream::getUnsigned(bits);
    if (!canBeUndefined || getMaxUnsignedValue(bits) != x) {
      return Optional<uint64_t>(x);
    }
  }
  return Optional<uint64_t>();
}

Optional<int64_t> Stream::getSigned(int bits, int64_t offset, bool canBeUndefined) {
  if (canRead(bits)) {
    auto x = getSigned(bits, offset);
    if (!canBeUndefined || getMaxSignedValue(bits, offset) != x) {
      return Optional<int64_t>(x);
    }
  }
  return Optional<int64_t>();
}

template <typename T>
Optional<double> toDouble(Optional<T> x) {
  if (x.defined()) {
    return Optional<double>(double(x()));
  }
  return Optional<double>();
}

Optional<double> Stream::getDouble(bool isSigned, int bits, int64_t offset, bool canBeUndefined) {
  return isSigned?
      toDouble(getSigned(bits, offset, canBeUndefined))
      : toDouble(getUnsigned(bits, canBeUndefined));

}

int64_t Stream::getSigned(int numBits, int64_t offset) {
  auto x = BitStream::getUnsigned(numBits);
  if (offset == 0) {
    if (isTwosComplementNegative(x, numBits)) {
    /*
       Suppose that we store the number -2 in 6 bits.
       The positive number 2 stored in 6 bits is

       000010

       The bits flipped:

       111101

       Add one to the flipped bits. This is the two's complement representation of -2:

       111110

       But since we keep it in a 64 bit unsigned integer, we
       currently obtain, from getUnsigned, the unsigned value x with this contents:

       00000000 00000000 00000000 00000000 00000000 00000000 00000000 00111110 (a)

       So we need to set the first 64-6 = 58 bits to 1, in order to obtain the two's complement
       representation of -2 in 64 bits.
       The maximum unsigned value is in 6 bits
       111111, and in 64 bits, it is

       00000000 00000000 00000000 00000000 00000000 00000000 00000000 00111111 (b)

       If we flip the bits, we obtain the bits that we need to fill in, in order
       to convert -2 from 6 bits to 64 bits, so we get (c) as the bits of (b) flipped:

       11111111 11111111 11111111 11111111 11111111 11111111 11111111 11000000 (c)

       So by filling in those bits, we get -2 in 64 bits as the bitwise or of (c) and (a):

       11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111110 (d)
      */
      uint64_t completed = ~getMaxUnsignedValue(numBits) | x;
      return static_cast<int64_t>(completed);
    }
    return static_cast<int64_t>(x);
  } else { /*
             This case is easy: We get an unsigned non-negative value, to to which we add
             a negative offset in order to represent a negative number.
           */
    return x + offset;
  }
}

}
