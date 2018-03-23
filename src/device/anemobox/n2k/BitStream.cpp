#include <device/anemobox/n2k/BitStream.h>
#include <assert.h>
#include <iostream>

BitStream::BitStream(const uint8_t* data, size_t len)
  : data_(data), lenBytes_(len) { }

bool BitStream::canRead(int numBits) const {
  return numBits <= remainingBits();
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

void BitOutputStream::pushUnsigned(int numBits, uint64_t value0) {
  // Gradually shifted to the right as we are writing the bits
  uint64_t value = value0;

  for (int bitsWritten = 0; bitsWritten < numBits; ) {
    uint8_t firstByte = 0xFF & value;
    int bitsWrittenInIteration = writeBitsInByte(
        numBits - bitsWritten, firstByte);
    value = value >> bitsWrittenInIteration;
    bitsWritten += bitsWrittenInIteration;
  }
}
