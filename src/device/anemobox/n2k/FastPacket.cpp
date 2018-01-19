#include <device/anemobox/n2k/FastPacket.h>
#include <algorithm>
#include <server/common/math.h>
#include <server/common/logging.h>

namespace PgnClasses {

const int FRAME_COUNTER_MASK = 0x1F; // 5 bits LSB
const int SEQUENCE_COUNTER_MASK = 0xE0; // bits 7 to 5
const int SEQUENCE_COUNTER_SHIFT = 5; // bits 7 to 5
const int SEQUENCE_COUNTER_NUM_BITS = 3;

// inspired from http://www.auelectronics.com/forum/index.php?PHPSESSID=ns0k786e6jvacog3re04jh6e50&topic=421.msg1185#msg1185
//
//Byte 1:  
// b0 – b4 = 1 to 31, 5 bit frame counter,  b0  =LSB 
// b5  –  b7  = 3 -bit sequence counter which shall be the same value as in the
// first frame, b5 is the LSB of the counter. 
//Byte 2: (only for the first frame: Expected length in bytes of the entire payload)
FastPacketBuffer::ReceptionKey FastPacketBuffer::keyForPacket(
    const CanPacket& packet) {
  return
    (((int(packet.shortSrc) << SEQUENCE_COUNTER_NUM_BITS)
    + ((packet.data[0] & SEQUENCE_COUNTER_MASK) >> SEQUENCE_COUNTER_SHIFT)))
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
      reception =
        _buffer.insert(std::make_pair(key, ReceptionBuffer(packet))).first;
    }
    int frameNo = packet.data[0] & FRAME_COUNTER_MASK;
    if (frameNo == 0) {
      reception->second.expectedLength = packet.data[1];
      reception->second.packet.data.resize(reception->second.expectedLength);
    }

    // This is where, in the destination buffer, the data
    // should be inserted.
    int dstOffset = dataDstOffset(packet);

    // This is where, in the small incoming source buffer,
    // we should start reading the data
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

namespace {
  uint8_t encodeFirstByte(uint8_t seqCounter, uint8_t frameCounter) {
    return // Is this bit fiddling OK?
        (seqCounter << SEQUENCE_COUNTER_SHIFT)
        | (frameCounter & FRAME_COUNTER_MASK);
  }

  struct FastPacketSplitState {
    FastPacketSplitState(int s) : seqCounter(s) {}

    uint8_t seqCounter = 0;
    uint8_t frameCounter = 0;
    int bytesSent = 0;

    void initializeNewPacket(const CanPacket& src, CanPacket* dst) {
      dst->pgn = src.pgn;
      dst->longSrc = src.longSrc;
      dst->shortSrc = src.shortSrc;
      dst->data.assign(8, 0xFF);
      dst->data[0] = encodeFirstByte(seqCounter, frameCounter);
      frameCounter++;
    }

    int computePayloadSize(int maxPayload, const CanPacket& src) const {
      return std::min(int(maxPayload), int(src.data.size() - bytesSent));
    }

    void sendBytes(
        int count,
        int dstOffset,
        const CanPacket& src, CanPacket* dst) {
      auto srcBegin = src.data.begin() + bytesSent;
      auto srcEnd = srcBegin + count;
      auto dstBegin = dst->data.begin() + dstOffset;
      std::copy(srcBegin, srcEnd, dstBegin);
      bytesSent += count;
    }

    void makeFirstPacket(const CanPacket& src, CanPacket* dst) {
      initializeNewPacket(src, dst);
      int n = computePayloadSize(6, src);
      dst->data[1] = src.data.size(); // Expected bytes (only first message)
      sendBytes(n, 2, src, dst);
    }

    void makeRemainingPacket(const CanPacket& src, CanPacket* dst) {
      initializeNewPacket(src, dst);
      int n = computePayloadSize(7, src);
      sendBytes(n, 1, src, dst);
    }

    bool remainsDataToBeSent(const CanPacket& src) const {
      return bytesSent < src.data.size();
    }
  };
}

std::vector<CanPacket> FastPacketSplitter::split(const CanPacket& src) {
  FastPacketSplitState state(_pgn2seqCounter[src.pgn].next());
  std::vector<CanPacket> dst;
  int packetCount = sail::div1(src.data.size()+1, 7);
  dst.resize(packetCount);
  state.makeFirstPacket(src, &(dst[0]));
  using namespace sail;
  for (int i = 1; i < packetCount; i++) {
    CHECK(state.remainsDataToBeSent(src));
    state.makeRemainingPacket(src,&(dst[i]));
  }
  CHECK(!state.remainsDataToBeSent(src));
  return dst;
}

}  // namespace PgnClasses
