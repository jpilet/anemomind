#include <device/anemobox/n2k/FastPacket.h>

#include <algorithm>

namespace PgnClasses {

const int FRAME_COUNTER_MASK = 0x1F; // 5 bits LSB
const int SEQUENCE_COUNTER_MASK = 0xE0; // bits 7 to 5

// inspired from http://www.auelectronics.com/forum/index.php?PHPSESSID=ns0k786e6jvacog3re04jh6e50&topic=421.msg1185#msg1185
//
//Byte 1:  
// b0 – b4 = 1 to 31, 5 bit frame counter,  b0  =LSB 
// b5  –  b7  = 3 -bit sequence counter which shall be the same value as in the
// first frame, b5 is the LSB of the counter. 
FastPacketBuffer::ReceptionKey FastPacketBuffer::keyForPacket(
    const CanPacket& packet) {
  return (((int(packet.shortSrc) << 3) + (packet.data[0] >> 5)))
    + (packet.pgn << 11);
}

int FastPacketBuffer::dataSrcOffset(const CanPacket& packet) {
  return (packet.data[0] & FRAME_COUNTER_MASK) == 0 ? 2 : 1;
}

int FastPacketBuffer::dataDstOffset(const CanPacket& packet) {
  int frameNo = (packet.data[0] & FRAME_COUNTER_MASK);
  const int FIRST_FRAME_PAYLOAD = 6;
  const int OTHER_FRAMES_PAYLOAD = 7;
  return (frameNo == 0 ?
    0 : FIRST_FRAME_PAYLOAD + (frameNo - 1) * OTHER_FRAMES_PAYLOAD);
}

void FastPacketBuffer::add(const CanPacket& packet) {
  if (packet.data.size() > 8) {
    // maybe already handled by J1939 layer.
    fullPacketReceived(packet);
  } else {
    ReceptionKey key = keyForPacket(packet);

    auto reception = _buffer.find(key);
    if (reception == _buffer.end()) {
      reception = _buffer.insert(std::make_pair(key, packet)).first;
      reception->second.packet.data.clear();
      reception->second.expectedLength = 0;
    }
    int frameNo = packet.data[0] & FRAME_COUNTER_MASK;
    if (frameNo == 0) {
      reception->second.expectedLength = packet.data[1];
      reception->second.packet.data.resize(reception->second.expectedLength);
    }
    int dstOffset = dataDstOffset(packet);
    int srcOffset = dataSrcOffset(packet);
    int newLength = dstOffset + packet.data.size() - srcOffset;
    if (reception->second.expectedLength) {
      newLength = std::min(int(reception->second.expectedLength), newLength);
    }
    if (newLength > reception->second.packet.data.size()) {
      reception->second.packet.data.resize(newLength);
    }
    int bytesToCopy =
      std::min(packet.data.size() - srcOffset,
               reception->second.packet.data.size() - dstOffset);

    std::copy(packet.data.begin() + srcOffset,
              packet.data.begin() + srcOffset + bytesToCopy,
              reception->second.packet.data.begin() + dstOffset);

      
    reception->second.framesReceived.set(frameNo);
    if (reception->second.isComplete()) {
      fullPacketReceived(reception->second.packet);
      _buffer.erase(reception);
    }
  }
}

}  // namespace PgnClasses
