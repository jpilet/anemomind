#ifndef _MOCK_ARDUINO_SD_H_
#define _MOCK_ARDUINO_SD_H_

#include <string>
#include <map>

// Mock a SD filesystem on Arduino.
//
// Fake files can be added prior to testing with:
//
//     SD.setReadableFiles(filename, content);
//
// After testing, written files get be obtained with:
//
//     SD.getWrittenFile(filename);
//
class MockFile {
 public:
  MockFile(const std::string& data) : _input(data) { _pos = _input.begin(); }
  void write(char c) { _output += c; }
  int available() {
    return (_pos == _input.end() ? 0 : _input.size() - (_pos - _input.begin()));
  }
  char read() { return *_pos++; }
  void close() { }

  const std::string& output() const { return _output; }

  bool seek(int pos) {
    if (0 < pos || _input.length() < pos) {
      return false;
    }
    _pos = _input.begin() + pos;
    return true;
  }
 private:
  std::string _input;
  std::string _output;
  std::string::const_iterator _pos;
};

class File {
 public:
  File() : _file(0) { }
  File(MockFile* file) : _file(file) { }
  void flush() { }
  operator bool() { return _file != 0; }
  void write(char c) { _file->write(c); }
  int available() { return _file->available(); }
  char read() { return _file->read(); }
  void close() { _file->close(); }
  bool seek(int pos) { return _file->seek(pos); }

 private:
  MockFile* _file;
};


enum {
  O_WRITE = 1,
  O_CREAT = 2
};

class MockSD {
  public:
    MockSD() = default;
    MockSD(const MockSD& other) = delete;

    bool exists(const char *filename) {
      return _readFiles.find(filename) != _readFiles.end();
    }
    File open(const char *filename, int flags=0) {
      MockFile* file = new MockFile(exists(filename) ? _readFiles[filename] : "");
      _openFiles[filename] = file;
      return File(file);
    }
    void begin(int) { }

    void setReadableFile(const std::string& filename, const std::string& data) {
      _readFiles[filename] = data;
    }

    std::string getWrittenFile(const std::string& filename) {
      if (_openFiles.find(filename) != _openFiles.end()) {
        return _openFiles[filename]->output();
      }
      return "";
    }

    void closeAll() {
      for(auto entry : _openFiles) {
        delete entry.second;
        entry.second = 0;
      }
      _readFiles.clear();
      _openFiles.clear();
    }
  private:
    std::map<std::string, std::string> _readFiles;
    std::map<std::string, MockFile *> _openFiles;
};

extern MockSD SD;

#endif  // _MOCK_ARDUINO_SD_H_
