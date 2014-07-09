#ifndef DEVICE_SCREEN_H
#define DEVICE_SCREEN_H

#include <Arduino.h>
#include <SoftwareSerial.h>

#define VERTICAL_SCREEN 0

// Initialize the screen.
void screenInit();

// Display a line. Valid lines: 0-3.
// the label is printed first, then the value.
void screenLine(int line, const char *label, int i);

// Display perf, twdir and tws.
void screenUpdate(int perf, int twdir, int tws);

// Display a debug number.
void screenUpdate(int i);

// Send raw command to the screen.
void screenSendData(const char *data);

#endif  // DEVICE_SCREEN_H
