#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <stdint.h>
#include <algorithm>

class BitStream {
 public:
  BitStream(uint8_t* data, size_t len);

  uint64_t getUnsigned(int numBits);
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

#endif  // BITSTREAM_H
