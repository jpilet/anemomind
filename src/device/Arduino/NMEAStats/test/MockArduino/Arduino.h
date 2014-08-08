#ifndef _MOCK_ARDUINO_ARDUINO_H_
#define _MOCK_ARDUINO_ARDUINO_H_

#define pinMode(a, b)

long millis();
long micros();
void delay(long ms);

template <typename T> T max(T a, T b) { return (a < b ? b : a); }
template <typename T> T min(T a, T b) { return (a < b ? a : b); }

#endif  // _MOCK_ARDUINO_ARDUINO_H_
