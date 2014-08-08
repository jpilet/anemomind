#ifndef _MOCK_ARDUINO_SOFTWARE_SERIAL_H_
#define _MOCK_ARDUINO_SOFTWARE_SERIAL_H_

#include <string>

class MockSerial {
  public:
    MockSerial() { _pos = _input.begin(); }
    void setData(const std::string& data) {
      _input = data;
      _pos = _input.begin();
    }
    void write(char c) { _output += c; }
    void begin(int) { }
    int available() { return (_pos != _input.end() ? 1 : 0); }
    char read() { return *_pos++; }

    std::string output() const { return _output; }
  private:
      std::string _input;
      std::string::const_iterator _pos;
      std::string _output;
};

extern MockSerial Serial;

class SoftwareSerial : public MockSerial {
  public:
    SoftwareSerial(int a, int b) { }
};

#endif  // _MOCK_ARDUINO_SOFTWARE_SERIAL_H_
