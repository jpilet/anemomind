#include <device/Arduino/libraries/ChunkFile/ChunkFile.h>

#include <gtest/gtest.h>
#include <assert.h>
#include <stdint.h>
#include <sstream>

#pragma pack(1)
struct TestStructA {
  enum {
    VERSION = 1,
    STRUCT_IDENTIFIER = 0x1342,
  };

  float b;
  int16_t a;
};

#pragma pack(1)
struct TestStructB {
  enum {
    VERSION = 1,
    STRUCT_IDENTIFIER = 0x1343,
  };

  int16_t a;
};

#pragma pack(1)
struct IgnoredStruct {
  enum {
    VERSION = 1,
    STRUCT_IDENTIFIER = 0x1111,
  };

  char data[16];
};

TEST(ChunkFileTest, SerializationTest) {
  TestStructA dataA;
  dataA.a = 23957;
  dataA.b = 2.456e-7;

  TestStructB dataB, dataC;
  dataB.a = 1234;
  dataC.a = 777;

  IgnoredStruct ignored;

  std::stringstream stream;
  writeChunk(stream, &dataB);
  writeChunk(stream, &ignored);
  writeChunk(stream, &dataA);
  writeChunk(stream, &dataC);

  TestStructA loadedA;
  TestStructB loadedB;
  TestStructB loadedC;

  ChunkTarget targets[] = {
    makeChunkTarget(&loadedA),
    makeChunkTarget(&loadedB),
    makeChunkTarget(&loadedC),
  };
  EXPECT_FALSE(targets[0].success);
  EXPECT_FALSE(targets[1].success);

  ChunkLoader loader(targets, 3);

  std::string str = stream.str();
  loader.addBytes(str.c_str(), str.size());

  EXPECT_EQ(dataA.a, loadedA.a);
  EXPECT_EQ(dataA.b, loadedA.b);
  EXPECT_EQ(dataB.a, loadedB.a);
  EXPECT_EQ(dataC.a, loadedC.a);
  EXPECT_TRUE(targets[0].success);
  EXPECT_TRUE(targets[1].success);
}
