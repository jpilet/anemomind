#ifndef DEVICE_CHUNK_LOADER_H
#define DEVICE_CHUNK_LOADER_H

#include <stdint.h>

struct ChunkTarget {
  void *destination;
  int16_t size;
  int16_t identifier;
  int16_t version;
  bool success;
};

template <class T>
ChunkTarget makeChunkTarget(T *destination) {
  ChunkTarget result;
  result.destination = destination;
  result.size = sizeof(T);
  result.identifier = T::STRUCT_IDENTIFIER;
  result.version = T::VERSION;
  result.success = false;
  return result;
}

class ChunkLoader {
 public:
  ChunkLoader(ChunkTarget *targets, int numTargets);

  void addBytes(const char *bytes, int numBytes);
  void addByte(char x);

 private:
  int _writePosition;
  int _bytesToSkip;

  ChunkTarget *_targets;
  int _numTargets;
  ChunkTarget *_currentTarget;

  char _chunkHeader[6]; // identifier, version, size. 2 bytes each.
  char _chunkHeaderPos;

  enum State {
    READ_CHUNK_HEADER,
    SKIP_CHUNK,
    READ_CHUNK,
  };
  State _state;
};

template <class Stream>
void writeInt16(Stream& stream, int16_t x) {
  stream << char(x & 0xFF) << char(x >> 8);
}

template <class Struct, class Stream>
void writeChunk(Stream& stream, const Struct *data) {
  writeInt16(stream, Struct::STRUCT_IDENTIFIER);
  writeInt16(stream, Struct::VERSION);
  writeInt16(stream, int16_t(sizeof(Struct)));

  for (int i = 0; i < sizeof(Struct); ++i) {
    const char *ptr = reinterpret_cast<const char *>(data);
    stream << ptr[i];
  }
}


#endif // DEVICE_CHUNK_LOADER_H
