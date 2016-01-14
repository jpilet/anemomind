#include <device/anemobox/n2k/BitStream.h>
#include <assert.h>

BitStream::BitStream(uint8_t* data, size_t len)
  : data_(data), len_(len), bitPos_(0) { }

bool BitStream::canRead(int numBits) const {
  return (bitPos_ + numBits) <= (len_ * 8);
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



