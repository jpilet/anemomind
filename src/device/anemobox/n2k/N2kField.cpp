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

bool contains(const std::initializer_list<int> &set, int x) {
  assert(std::is_sorted(set.begin(), set.end()));
  auto ptr = std::lower_bound(set.begin(), set.end(), x);
  if (ptr == set.end()) {
    return false;
  } else {
    return (*ptr) == x;
  }
}


Optional<uint64_t> N2kFieldStream::getUnsigned(int bits, Definedness d) {
  if (canRead(bits)) {
    auto x = BitStream::getUnsigned(bits);
    auto invalid = getMaxUnsignedValue(bits);
    if (d == Definedness::AlwaysDefined
        || (invalid != x && (invalid - 1) != x && (invalid - 2) != x)) {
      return Optional<uint64_t>(x);
    }
  }
  return Optional<uint64_t>();
}

Optional<int64_t> N2kFieldStream::getSigned(int bits, int64_t offset, Definedness d) {
  if (canRead(bits)) {
    auto x = getSigned(bits, offset);
    if (d == Definedness::AlwaysDefined || getMaxSignedValue(bits, offset) != x) {
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

Optional<double> N2kFieldStream::getDouble(bool isSigned, int bits, int64_t offset, Definedness d) {
  return isSigned?
      toDouble(getSigned(bits, offset, d))
      : toDouble(getUnsigned(bits, d));

}

Optional<uint64_t> N2kFieldStream::getUnsignedInSet(int numBits, const std::initializer_list<int> &set) {
  auto x = getUnsigned(numBits, Definedness::AlwaysDefined);
  if (x.defined()) {
    if (contains(set, x())) {
      return x;
    }
  }
  return Optional<uint64_t>();
}

sail::Array<uint8_t> N2kFieldStream::readBytes(int numBits) {
  int byteCount = numBits/8;
  if (numBits == byteCount*8) {
    sail::Array<uint8_t> dst(byteCount);
    for (int i = 0; i < byteCount; i++) {
      dst[i] = getByte();
    }
    return dst;
  } else {
    // Only read whole bytes.
    advanceBits(numBits);

    // Hm... so we advance the bits but don't return anything?
    // Looking at 'gen.js', there is an assertion already at code generation
    // to check that we only read whole bytes. So we should never end up
    // in this branch.
    return sail::Array<uint8_t>();
  }
}



int64_t N2kFieldStream::getSigned(int numBits, int64_t offset) {
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

void N2kFieldOutputStream::pushUnsigned(int bits, Optional<uint64_t> value) {
  auto invalid = getMaxUnsignedValue(bits);
  if (value.defined()) {
    if (value.get() < invalid) {
      _dst.pushUnsigned(bits, value.get());
      return;
    }
  }
  _dst.pushUnsigned(bits, invalid);
}

void N2kFieldOutputStream::pushSigned(
    int bits, int64_t offset, Optional<int64_t> value) {

  // Is it really this easy? Or did we miss anything regarding endianness?
  _dst.pushUnsigned(bits, static_cast<uint64_t>(
      value.defined()?
          (value.get() - offset)
          : getMaxSignedValue(bits, offset)));
}

void N2kFieldOutputStream::pushDouble(
    bool isSigned, int bits,
    int64_t offset, Optional<double> value) {
  if (isSigned) {
    pushSigned(bits, offset, value.defined()?
        Optional<int64_t>(value.get())
        : Optional<int64_t>());
  } else {
    pushUnsigned(bits, value.defined()?
        Optional<uint64_t>(value.get())
        : Optional<uint64_t>());
  }
}


}
