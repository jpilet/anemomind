/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef DEVICE_ANEMOBOX_N2K_N2KFIELD_H_
#define DEVICE_ANEMOBOX_N2K_N2KFIELD_H_

#include <device/anemobox/n2k/BitStream.h>
#include <server/common/Optional.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <bitset>

namespace N2kField {

constexpr inline size_t size_t_Power(size_t a, size_t b) {
  return (b <= size_t(0)? size_t(1) : a*size_t_Power(a, b - 1));
}

enum class Definedness {
  AlwaysDefined,
  UndefinedIfMax
};

uint64_t getMaxUnsignedValue(int numBits);
uint64_t getMaxSignedValue(int numBits, int64_t offset);

class N2kFieldStream : public BitStream {
 public:
  N2kFieldStream(const uint8_t *data, int lengthBytes) : BitStream(data, lengthBytes) {}

  Optional<uint64_t> getUnsigned(int bits, Definedness definedness);
  Optional<int64_t> getSigned(int bits, int64_t offset, Definedness definedness);
  Optional<double> getDouble(bool isSigned, int bits, int64_t offset, Definedness definedness);

  template <typename T>
  Optional<T> getPhysicalQuantity(bool isSigned, double resolution, T unit, int bits, int64_t offset) {
    auto x = getDouble(isSigned, bits, offset, Definedness::UndefinedIfMax);
    if (x.defined()) {
      return Optional<T>(x()*resolution*unit);
    }
    return Optional<T>();
  }

  // This function can serve as a base for reading
  // enum'ed values. The 'set' parameter defines what
  // integers that belong to the set of enum'ed values.
  template <int NumBits>
  Optional<uint64_t> getUnsignedInSet(
      const std::bitset<size_t_Power(2, NumBits)> &set) {
    auto x = getUnsigned(NumBits, Definedness::AlwaysDefined);
    if (x.defined()) {
      if (set[x()]) {
        return x;
      }
    }
    return Optional<uint64_t>();
  }
 private:
  int64_t getSigned(int numBits, int64_t offset);
};


}


#endif /* DEVICE_ANEMOBOX_N2K_N2KFIELD_H_ */
