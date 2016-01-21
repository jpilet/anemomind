#include <device/anemobox/n2k/BitStream.h>
#include <assert.h>
#include <iostream>

BitStream::BitStream(const uint8_t* data, size_t len)
  : data_(data), lenBytes_(len), bitPos_(0) { }

bool BitStream::canRead(int numBits) const {
  return (bitPos_ + numBits) <= (lenBytes_ * 8);
}

bool BitStream::isAvailable(int64_t x, int numBits) {
  assert(false);
  auto maxv = static_cast<int64_t>(BitStreamInternals::getMaxSignedValueTwosComplement(numBits));
  return x != maxv;
}

bool BitStream::isAvailable(uint64_t x, int numBits) {
  auto maxv = BitStreamInternals::getMaxUnsignedValue(numBits);
  return x != maxv;
}

uint64_t BitStream::getUnsigned(int numBits) {
  assert(numBits > 0 && numBits <= 64);
  assert(canRead(numBits));

  uint64_t result = 0;

  for (int bitRead = 0; bitRead < numBits; ) {
    uint8_t bits;
    int newBits = readBitsInByte(numBits - bitRead, &bits);
    result |= uint64_t(bits) << bitRead;
    bitRead += newBits;
  }

  return result;
}

namespace BitStreamInternals {
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
}

int64_t BitStream::getSigned(int numBits, int64_t offset) {
  using namespace BitStreamInternals;
  auto x = getUnsigned(numBits);
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




