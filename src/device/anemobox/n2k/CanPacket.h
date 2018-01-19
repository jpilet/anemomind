#ifndef N2K_CANPACKET_H
#define N2K_CANPACKET_H

#include <string>
#include <stdint.h>
#include <vector>

namespace PgnClasses {

struct CanPacket {
  std::string longSrc;
  uint8_t shortSrc = 0;
  int pgn = 0;
  std::vector<uint8_t> data;

  CanPacket() {}
  CanPacket(
      const std::string& ls,
      uint8_t ss, int p,
      const std::vector<uint8_t>& d)
    : longSrc(ls), shortSrc(ss),
      pgn(p), data(d) {}
};

}  // namespace PgnClasses

#endif  // N2K_CANPACKET_H
