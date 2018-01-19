#include <device/anemobox/n2k/FastPacket.h>

#include <algorithm>
#include <gtest/gtest.h>

using namespace PgnClasses;

class FastPacketTester : public FastPacketBuffer {
  public:
    FastPacketTester(uint8_t* result, int resultLength)
      : _result(result), _resultLength(resultLength), _numCalls(0) { }

    int numCalls() const { return _numCalls; }

  protected:
    void fullPacketReceived(const CanPacket& fullpacket) {
      EXPECT_EQ(_resultLength, fullpacket.data.size());
      for (int i = 0; i < _resultLength; ++i) {
        EXPECT_EQ(_result[i], fullpacket.data[i]);
      }
      ++_numCalls;
    }

  private:
    uint8_t* _result;
    int _resultLength;
    int _numCalls;
};

uint8_t testInputData[7][8] = {
 { 0x60, 0x2b, 0x00, 0x74, 0x40, 0xc0, 0xca, 0x97 },
 { 0x61, 0x0a, 0x00, 0x10, 0xff, 0xed, 0xf5, 0x21 },
 { 0x62, 0xe5, 0x05, 0x00, 0xa9, 0xf1, 0x8f, 0xf6 },
 { 0x63, 0x88, 0x16, 0xf6, 0x90, 0xba, 0xc5, 0x03 },
 { 0x64, 0x00, 0x00, 0x00, 0x00, 0x12, 0xfc, 0x00 },
 { 0x65, 0xbe, 0x00, 0x54, 0x01, 0xe1, 0xf2, 0xff },
 { 0x66, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff }
};

uint8_t testResult[] = {
       0x00, 0x74, 0x40, 0xc0, 0xca, 0x97,
 0x0a, 0x00, 0x10, 0xff, 0xed, 0xf5, 0x21,
 0xe5, 0x05, 0x00, 0xa9, 0xf1, 0x8f, 0xf6,
 0x88, 0x16, 0xf6, 0x90, 0xba, 0xc5, 0x03,
 0x00, 0x00, 0x00, 0x00, 0x12, 0xfc, 0x00,
 0xbe, 0x00, 0x54, 0x01, 0xe1, 0xf2, 0xff,
 0xff, 0x00
};

CanPacket getInputPacket(int i) {
  CanPacket packet;
  packet.shortSrc = 7;
  packet.pgn = 129029;
  packet.data.resize(8);
  
  std::copy(testInputData[i], testInputData[i]+8, packet.data.begin());

  return packet;
}

TEST(FastPacketTest, BasicTest) {
  EXPECT_EQ(testInputData[0][1], sizeof(testResult));

  FastPacketTester fastPacketBuffer(testResult, sizeof(testResult));

  for (int i = 0; i < 7; ++i) {
    fastPacketBuffer.add(getInputPacket(i));
  }

  EXPECT_EQ(1, fastPacketBuffer.numCalls());
}

TEST(FastPacketTest, OutOfOrderTest) {
  EXPECT_EQ(testInputData[0][1], sizeof(testResult));

  FastPacketTester fastPacketBuffer(testResult, sizeof(testResult));

  fastPacketBuffer.add(getInputPacket(3));
  fastPacketBuffer.add(getInputPacket(2));
  fastPacketBuffer.add(getInputPacket(5));
  fastPacketBuffer.add(getInputPacket(6));
  fastPacketBuffer.add(getInputPacket(0));
  fastPacketBuffer.add(getInputPacket(1));

  EXPECT_EQ(0, fastPacketBuffer.numCalls());
  fastPacketBuffer.add(getInputPacket(4));

  EXPECT_EQ(1, fastPacketBuffer.numCalls());
}

TEST(FastPacketTest, DuplicatePacketTest) {
  EXPECT_EQ(testInputData[0][1], sizeof(testResult));

  FastPacketTester fastPacketBuffer(testResult, sizeof(testResult));

  fastPacketBuffer.add(getInputPacket(5));
  fastPacketBuffer.add(getInputPacket(6));
  fastPacketBuffer.add(getInputPacket(3));
  fastPacketBuffer.add(getInputPacket(2));
  fastPacketBuffer.add(getInputPacket(5));
  fastPacketBuffer.add(getInputPacket(6));
  fastPacketBuffer.add(getInputPacket(0));
  fastPacketBuffer.add(getInputPacket(6));
  fastPacketBuffer.add(getInputPacket(0));
  fastPacketBuffer.add(getInputPacket(1));

  EXPECT_EQ(0, fastPacketBuffer.numCalls());
  fastPacketBuffer.add(getInputPacket(4));

  EXPECT_EQ(1, fastPacketBuffer.numCalls());
}

int getSeqCounter(uint8_t firstByte) {
  return (firstByte >> 5) & 0x07;
}

int getFrameCounter(uint8_t firstByte) {
  return (firstByte & 0x1F);
}

TEST(FastPacketSplitterTest, DecodeIt) {
  FastPacketSplitter splitter;
  auto fullPacket = getInputPacket(0);
  fullPacket.data.resize(sizeof(testResult));
  std::copy(testResult, testResult + sizeof(testResult),
      fullPacket.data.data());

  std::set<int> seqCounterValues;

  for (int k = 0; k < 12; k++) {
    auto packets = splitter.split(fullPacket);
    EXPECT_EQ(7, packets.size());


    // The seqCounter is not necessarily that of the test data,
    // the important thing is that all split packets have the
    // same seqCounter.
    int commonSeqCounter = -119;

    for (int i = 0; i < packets.size(); i++) {
      auto expected = getInputPacket(i);
      const auto& p = packets[i];
      auto firstByte = p.data[0];
      EXPECT_EQ(i, getFrameCounter(firstByte));
      if (i == 0) {
        EXPECT_EQ(p.data[1], sizeof(testResult));
        commonSeqCounter = getSeqCounter(firstByte);
        EXPECT_LE(0, commonSeqCounter);
        EXPECT_LT(commonSeqCounter, 8);
      } else {
        EXPECT_EQ(commonSeqCounter, getSeqCounter(firstByte));
      }
      EXPECT_EQ(expected.pgn, p.pgn);
      EXPECT_EQ(expected.shortSrc, p.shortSrc);
      EXPECT_EQ(expected.longSrc, p.longSrc);
      int n = expected.data.size();
      EXPECT_EQ(n, p.data.size());

      int dataStart = i == 0? 2 : 1;
      for (int j = dataStart; j < n; j++) {
        EXPECT_EQ(p.data[j], expected.data[j]);
      }
    }
    seqCounterValues.insert(commonSeqCounter);

    EXPECT_EQ(seqCounterValues.size(), 1+std::min(7, k));
  }

  for (auto v: seqCounterValues) {
    EXPECT_LE(0, v);
    EXPECT_LT(v, 8);
  }


}

