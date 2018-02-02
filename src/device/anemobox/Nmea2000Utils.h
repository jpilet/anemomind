/*
 * BasicNMEA2000.h
 *
 *  Created on: 26 Jan 2018
 *      Author: jonas
 */

#ifndef DEVICE_ANEMOBOX_BASICNMEA2000_H_
#define DEVICE_ANEMOBOX_BASICNMEA2000_H_

#include <NMEA2000.h>
#include <vector>
#include <queue>
#include <server/common/logging.h>
#include <server/common/Optional.h>

namespace sail {

template <typename Iter>
void setN2kMsgData(Iter begin, Iter end, tN2kMsg* dst) {
  std::copy(begin, end, dst->Data);
  auto n = std::distance(begin, end);
  CHECK(n < dst->MaxDataLen);
  dst->DataLen = n;
}

// Safe construction of N2kMsg
// (no risk of calling things in the wrong order,
//  making sure all things are defined, etc.)
// Without this class, there is a risk that we set data
// of the N2kMsg instance before having called msg.Init.
// (I had that bug). Or that we mixup the order of the
// integer arguments that msg.Init takes.
struct N2kMsgBuilder {
  static const int defaultPriority = 6;
  unsigned char priority = defaultPriority; // Seems to be a common value.

  // Probably none of these have good default values
  Optional<unsigned long> PGN;
  Optional<unsigned char> source;
  Optional<unsigned char> destination;

  template <typename Collection>
  tN2kMsg make(const Collection& srcData) const {
    auto msg = initialize();
    setN2kMsgData(srcData.begin(), srcData.end(), &msg);
    return msg;
  }

  tN2kMsg makeFromArray(int N, uint8_t* srcData) const {
    auto msg = initialize();
    setN2kMsgData(srcData, srcData + N, &msg);
    return msg;
  }
private:
  tN2kMsg initialize() const {
    CHECK(PGN.defined());
    CHECK(source.defined());
    CHECK(destination.defined());
    tN2kMsg msg;
    msg.Init(
        priority, PGN.get(),
        source.get(), destination.get());
    return msg;
  }
};

} /* namespace sail */

#endif /* DEVICE_ANEMOBOX_BASICNMEA2000_H_ */
