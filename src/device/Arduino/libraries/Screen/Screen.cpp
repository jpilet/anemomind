#include "Screen.h"

#ifdef ARDUINO_UNO
SoftwareSerial screenSerial(8, 9); // RX, TX
#else
#define screenSerial Serial1
#endif

void screenLine(int line, const char *label, int i) {
  char str[32];
  sprintf(str,
          (VERTICAL_SCREEN ? "#ZC%d,90,%s%03d \r" : "#ZC0,%d,%s %3d  \r"),
          (VERTICAL_SCREEN ? line * 30 : line * 22),
          label,
          i);
  screenSendData(str);
}

void screenUpdate(int perf, int twdir, int tws) {
  screenLine(0, (VERTICAL_SCREEN? "p" : "PERFMG"), perf);
  screenLine(1, (VERTICAL_SCREEN? "d" : "TWDIR"), (twdir < 0 ? twdir + 360 : twdir));
  screenLine(2, (VERTICAL_SCREEN? "s" : "TWS"), tws);
}

void screenUpdate(int i) {
  screenLine(1, "", i);
}

void screenSendData(const char *data) {
  unsigned char i, bcc;
  screenSerial.write(0x11);
  bcc = 0x11;
  int len = strlen(data);
  screenSerial.write(len);
  bcc = bcc + len;
  for(i=0; i < len; i++) {
    screenSerial.write(data[i]);
    bcc = bcc + data[i];
  }
  screenSerial.write(bcc);
  delay(2);
}

void screenInit() {
  screenSerial.begin(115200);
  
  delay(3);

  // Disable terminal mode
  screenSendData("#TA,");
  
  // Clear screen
  screenSendData("#DL,");
  
  if (0) {
    // Turn backlight off
    screenSendData("#YL0,");
  }
  
  if (VERTICAL_SCREEN) {
    // text orientation vertical
    screenSendData("#ZW1,");
    // Font selection
    screenSendData("#ZF6,");
  } else {
    // Font selection
    screenSendData("#ZF4,");
    // zoom factor
    screenSendData("#ZZ2,2,");
  }
}
