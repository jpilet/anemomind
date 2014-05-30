#include "ChunkFile.h"

namespace {

int16_t makeInt16(char bytes[2]) {
  return int16_t(bytes[0]) + (int16_t(bytes[1]) << 8);
}

} // namespace

ChunkLoader::ChunkLoader(ChunkTarget *targets, int numTargets) :
  _targets(targets),
  _numTargets(numTargets),
  _chunkHeaderPos(0),
  _state(READ_CHUNK_HEADER) {
}

void ChunkLoader::addByte(char x) {
  switch (_state) {
    case READ_CHUNK_HEADER:
      _chunkHeader[_chunkHeaderPos] = x;
      ++_chunkHeaderPos;
      if (_chunkHeaderPos == 6) {
        int16_t id = makeInt16(_chunkHeader + 0);
        int16_t version = makeInt16(_chunkHeader + 2);
        int16_t size = makeInt16(_chunkHeader + 4);

        _state = SKIP_CHUNK;
        _bytesToSkip = size;

        for (int i = 0; i < _numTargets; ++i) {
          if (id == _targets[i].identifier
              && version == _targets[i].version
              && size == _targets[i].size) {
            _currentTarget = _targets + i;
            _state = READ_CHUNK;
            _writePosition = 0;
          }
        }
      }
      break;

    case SKIP_CHUNK:
      --_bytesToSkip;
      if (_bytesToSkip == 0) {
        _state = READ_CHUNK_HEADER;
        _chunkHeaderPos = 0;
      }
      break;

    case READ_CHUNK:
      reinterpret_cast<char *>(_currentTarget->destination)[_writePosition] = x;
      _writePosition++;
      if (_writePosition >= _currentTarget->size) {
        _currentTarget->success = true;
        _state = READ_CHUNK_HEADER;
        _chunkHeaderPos = 0;
      }
      break;
  }
}

void ChunkLoader::addBytes(const char *bytes, int numBytes) {
  for (int i = 0; i < numBytes; ++i) {
    addByte(bytes[i]);
  }
}
