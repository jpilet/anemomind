#ifndef N2K_FASTPACKET_H
#define N2K_FASTPACKET_H

#include <bitset>
#include <map>
#include <device/anemobox/n2k/CanPacket.h>

namespace PgnClasses {

class FastPacketBuffer {
  public:

    void add(const CanPacket& packet);

  protected:
    virtual void fullPacketReceived(const CanPacket& fullPacket) = 0;

  private:
    typedef int ReceptionKey;
    struct ReceptionBuffer {
      CanPacket packet;
      std::bitset<32> framesReceived;
      uint8_t expectedLength;

      ReceptionBuffer(const CanPacket& packet) : packet(packet) { }

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

}  // namespace PgnClasses

#endif   // N2K_FASTPACKET_H
