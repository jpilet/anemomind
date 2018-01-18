#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <stdint.h>
#include <algorithm>
#include <vector>

// Helper class for the classes below
class BitCounter {
public:
  int bitPos() const { return bitPos_; }

  int advanceBits(int bits) { bitPos_ += bits; return bitPos_; }

  int bytePos() const { return bitPos_ / 8; }
  int bitOffset() const { return bitPos_ % 8; }

  int remainingBitsInThisByte() const {
    return 8 - bitOffset();
  }
private:
  int bitPos_ = 0;
};

inline uint8_t maskForUnsignedNumber(int actualNumBits) {
  return ((unsigned(1) << actualNumBits) - 1);
}

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
 *
 * TODO: Consider renaming it to BitInputStream.
 */
class BitStream {
 public:
  BitStream(const uint8_t* data, size_t len);

  uint64_t getUnsigned(int numBits);
  uint8_t getByte() { return uint8_t(getUnsigned(8)); }
  bool canRead(int numBits) const; 

  int advanceBits(int n) {return _counter.advanceBits(n);}

  int lengthBytes() const {
    return lenBytes_;
  }

  int lengthBits() const {
    return 8*lenBytes_;
  }

  int remainingBits() const {
    return lengthBits() - _counter.bitPos();
  }

 private:
  BitCounter _counter;

  // Returns the number of bits actually read
  uint8_t readBitsInByte(
      int wantedNumBits, /** <[in] How many bits to read */
      uint8_t* bits /** <[out] A single byte containing the read bits */) {
    int remainingBitsInThisByte = _counter.remainingBitsInThisByte();
    int actualNumBits = std::min(wantedNumBits, remainingBitsInThisByte);
    *bits = unsigned(data_[_counter.bytePos()] >> _counter.bitOffset())
      & maskForUnsignedNumber(actualNumBits); // Masking only the bits that we want
    _counter.advanceBits(actualNumBits);
    return actualNumBits;
  }

  const uint8_t* data_;
  size_t lenBytes_;
};

class BitOutputStream {
public:
  // Does the opposite of what BitStream::getUnsigned does
  void pushUnsigned(int bitCount, uint64_t value);

  int advanceBits(int n) {return _counter.advanceBits(n);}

  int lengthBits() const {return _counter.bitPos();}

  const std::vector<uint8_t>& data() const {return _data;}
private:
  // Does the opposite of what BitStream::readBitsInByteDoes.
  uint8_t writeBitsInByte(
      int wantedNumBits, /** <[in] How many bits we would like to write*/
      uint8_t inputByte /** <[in] The byte that we will try to write*/) {
    while (!(_counter.bytePos() < _data.size())) {
      _data.push_back(0xFF); // Should unused bits be 1 by default?
    }
    int remainingBitsInThisByte = _counter.remainingBitsInThisByte();
    int actualNumBits = std::min(wantedNumBits, remainingBitsInThisByte);
    int offset = _counter.bitOffset();
    auto mask = maskForUnsignedNumber(actualNumBits);
    _data[_counter.bytePos()] &=
        // The bits of interest at their right location in the target byte
        (inputByte << offset)

        // Set the remaining bits to 1, because we are and'ing.
        | ~(mask << offset);

    _counter.advanceBits(actualNumBits);
    return actualNumBits;
  }

  BitCounter _counter;
  std::vector<uint8_t> _data;
};

#endif  // BITSTREAM_H
