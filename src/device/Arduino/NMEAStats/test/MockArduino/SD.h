#ifndef _MOCK_ARDUINO_SD_H_
#define _MOCK_ARDUINO_SD_H_

class File {
  public:
    void flush() { }
    operator bool() { return true; }
    void write(char c) { }
    int available() { return 0; }
    char read() { return 0; }
    void close() { }
};

class MockSD {
  public:
    bool exists(const char *filename) { return false; }
    File open(const char *filename, int flags=0) { return File(); }
    void begin(int) { }
};

extern MockSD SD;

enum {
  O_WRITE = 1,
  O_CREAT = 2
};

#endif  // _MOCK_ARDUINO_SD_H_
