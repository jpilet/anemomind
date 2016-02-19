#ifndef N2K_CANPACKET_H
#define N2K_CANPACKET_H

#include <string>
#include <stdint.h>
#include <vector>

namespace PgnClasses {

struct CanPacket {
  std::string longSrc;
  uint8_t shortSrc;
  int pgn;
  std::vector<uint8_t> data;
};

}  // namespace PgnClasses

#endif  // N2K_CANPACKET_H
