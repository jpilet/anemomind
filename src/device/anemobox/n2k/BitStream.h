#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <stdint.h>
#include <algorithm>

/* Read a packed stream of arbirarily sized ints, not necessarily aligned on
 * bytes.
 *
 * The encoding is "little endian": to encode 3 bits in a byte, the 3 low bits
 * are used first.
 * Thus, the stream of 4-bit int: 0x1, 0x2, 0x3, 0x4 is encoded:
 * 0x21, 0x43
 *
 * It is possible to read up to 64 bits in a single read. A 64bit int is
 * assumed to be encoded little endian.
 */
class BitStream {
 public:
  BitStream(uint8_t* data, size_t len);

  uint64_t getUnsigned(int numBits);
  int64_t getSigned(int numBits, int64_t offset = 0);
  uint8_t getByte() { return uint8_t(getUnsigned(8)); }
  bool canRead(int numBits) const; 

  int bitPos() const { return bitPos_; }
  int bytePos() const { return bitPos_ / 8; }
  int bitOffset() const { return bitPos_ % 8; }

 private:
  uint8_t readBitsInByte(int wantedNumBits, uint8_t*bits) {
    int remainingBitsInThisByte = 8 - bitOffset();
    int actualNumBits = std::min(wantedNumBits, remainingBitsInThisByte);
    *bits = unsigned(data_[bytePos()] >> bitOffset())
      & ((unsigned(1) << actualNumBits) - 1);
    bitPos_ += actualNumBits;
    return actualNumBits;
  }

  uint8_t* data_;
  size_t len_;
  int bitPos_;
};

// Exposed for testability
namespace BitStreamInternals {

  // Check if a number that is assumged to be signed, is negative
  bool isTwosComplementNegative(uint64_t value, int numBits);

  // For example, if have 6 bits, this function should return 2^6-1, which is 63
  uint64_t getMaxUnsignedValue(int numBits);
  uint64_t getMaxSignedValueTwosComplement(int numBits);
}

#endif  // BITSTREAM_H
