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

int main() {
  TestStructA dataA;
  dataA.a = 23957;
  dataA.b = 2.456e-7;

  TestStructB dataB;
  dataB.a = 1234;

  std::stringstream stream;
  writeChunk(stream, &dataB);
  writeChunk(stream, &dataA);

  TestStructA loadedA;
  TestStructB loadedB;

  ChunkTarget targets[] = {
    makeChunkTarget(&loadedA),
    makeChunkTarget(&loadedB),
  };
  ChunkLoader loader(targets, 2);

  std::string str = stream.str();
  loader.addBytes(str.c_str(), str.size());

  assert(dataA.a == loadedA.a);
  assert(dataA.b == loadedA.b);
  assert(dataB.a == loadedB.a);
  assert(targets[0].success);
  assert(targets[1].success);

  std::cout << "OK.\n";
  return 0;
}
