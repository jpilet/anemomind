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
#include <server/common/TimeStamp.h>

namespace N2kField {

template <typename PgnClass>
sail::TimeStamp getTimeStamp(const PgnClass &x) {
  if (x.date().defined() && x.time().defined()) {
    return sail::TimeStamp::offset1970() + x.date().get() + x.time().get();
  }
  return sail::TimeStamp();
}

bool contains(const std::initializer_list<int> &set, int x);

enum class Definedness {
  AlwaysDefined, // Seems like this is what we always use.
  MaybeUndefined
};

uint64_t getMaxUnsignedValue(int numBits);
uint64_t getMaxSignedValue(int numBits, int64_t offset);

/*
 * This class is used by the generated classes in PgnClasses.{h,cpp}
 */
class N2kFieldStream : public BitStream {
 public:
  N2kFieldStream(const uint8_t *data, int lengthBytes) : BitStream(data, lengthBytes) {}

  Optional<uint64_t> getUnsigned(int bits, Definedness definedness);
  Optional<int64_t> getSigned(int bits, int64_t offset, Definedness definedness);
  Optional<double> getDouble(bool isSigned, int bits, int64_t offset, Definedness definedness);

  template <typename T>
  Optional<T> getPhysicalQuantity(
      bool isSigned, double resolution,
      T unit, int bits, int64_t offset) {
    auto x = getDouble(isSigned, bits, offset, Definedness::MaybeUndefined);
    if (x.defined()) {
      return Optional<T>(x()*resolution*unit);
    }
    return Optional<T>();
  }

  // This function can serve as a base for reading
  // enum'ed values. The 'set' parameter defines what
  // integers that belong to the set of enum'ed values.
  Optional<uint64_t> getUnsignedInSet(int numBits, const std::initializer_list<int> &set);

  sail::Array<uint8_t> readBytes(int numBits);
 private:
  int64_t getSigned(int numBits, int64_t offset);
};

/*
 * This class does the opposite of what N2kFieldStream does
 */
class N2kFieldOutputStream  {
public:
  void pushUnsigned(int bits, Optional<uint64_t> value);
  void pushSigned(int bits, int64_t offset, Optional<int64_t> value);
  void pushDouble(
      bool isSigned, int bits,
      int64_t offset, Optional<double> value);

  template <typename T>
  void pushPhysicalQuantity(
      bool isSigned, double resolution,
      T unit, int bits, int64_t offset, Optional<T> value) {
    pushDouble(isSigned, bits, offset,
        value.defined()?
            Optional<double>((value.get()/unit)/resolution)
            : Optional<double>());
  }

  // No 'pushUnsignedInSet', just use 'pushUnsigned' for that.

  void writeBytes(const sail::Array<uint8_t>& bytes);
  const std::vector<uint8_t>& data() const {return _dst.data();}

  void skipBits(int n, bool fillValue) {_dst.skipBits(n, fillValue);}
private:
  BitOutputStream _dst;
};

}


#endif /* DEVICE_ANEMOBOX_N2K_N2KFIELD_H_ */
