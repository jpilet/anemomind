#include "../ChunkFile.h"

#include <assert.h>
#include <stdint.h>
#include <string>
#include <sstream>
#include <iostream>

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

int main() {
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
  ChunkLoader loader(targets, 3);

  std::string str = stream.str();
  loader.addBytes(str.c_str(), str.size());

  assert(dataA.a == loadedA.a);
  assert(dataA.b == loadedA.b);
  assert(dataB.a == loadedB.a);
  assert(dataC.a == loadedC.a);
  assert(targets[0].success);
  assert(targets[1].success);

  std::cout << "OK.\n";
  return 0;
}
