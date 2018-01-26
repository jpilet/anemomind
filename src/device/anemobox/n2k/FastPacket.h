#ifndef N2K_FASTPACKET_H
#define N2K_FASTPACKET_H

#include <bitset>
#include <map>
#include <device/anemobox/n2k/CanPacket.h>

namespace PgnClasses {

class FastPacketBuffer {
  public:

    void add(const CanPacket& packet);
    virtual ~FastPacketBuffer() {}
  protected:
    virtual void fullPacketReceived(const CanPacket& fullPacket) = 0;

  private:
    typedef int ReceptionKey;
    struct ReceptionBuffer {
      CanPacket packet;
      std::bitset<32> framesReceived;
      uint8_t expectedLength;

      ReceptionBuffer(const CanPacket& packet)
        : packet(packet), expectedLength(0) { this->packet.data.clear(); }

      bool isComplete() const { 
        if (!framesReceived.test(0)) {
          return false;
        }
        // this is a simpler way of expressing
        // int(ceil((expectedLength - 1) / 7.0))
        int expectedNumFrames = (expectedLength / 7) + 1;
        for (int i = 0; i < expectedNumFrames; ++i) {
          if (!framesReceived.test(i)) {
            return false;
          }
        }
        return true;
      }
    };

    static ReceptionKey keyForPacket(const CanPacket& packet);
    static int dataSrcOffset(const CanPacket& packet);
    static int dataDstOffset(const CanPacket& packet);

    std::map<ReceptionKey, ReceptionBuffer> _buffer;
};

/* Performs the opposite of the FastPacketBuffer class: Given a
   big packet, a so called "Fast packet", split it up
   into smaller packets.

   Note: Not all packets are "Fast packets".
   This is handled by the PgnClasses file. In particular, see
   the method PgnVisitor::pushAndLinkPacket. Given an incoming packet,
   that method determines whether it should be linked into a fast packet
   or if it is a single packet. TODO: We need to write the opposite of
   that in the code generator.
   */
class FastPacketSplitter {
public:
  std::vector<std::vector<uint8_t>> split(
      int pgn, const std::vector<uint8_t>& src);
private:
  struct FastPacketSeqCounter {
    uint8_t value = 0;
    uint8_t next() {value = (value + 1) % 8; return value;}
  };

  std::map<int, FastPacketSeqCounter> _pgn2seqCounter;
};

}  // namespace PgnClasses

#endif   // N2K_FASTPACKET_H
