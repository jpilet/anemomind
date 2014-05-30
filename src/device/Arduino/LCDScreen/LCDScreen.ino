#include <SoftwareSerial.h>
#define VERTICAL_SCREEN 1

/*
 DIP128W-6LW
 Example Code
 
 xavier@thefabric.com
 */

SoftwareSerial mySerial(10, 11); // RX, TX

void setup() {
  Serial.begin(57600);
  mySerial.begin(115200);
  initScreen();
  Serial.println("DIP128W-6LW Example Code running...");
}

void loop() {
  for (int i=10; i<100; i++) {
    updateDisplay(i);
    delay(300); 
  }
}

void updateDisplay(int i) {
  if (VERTICAL_SCREEN) {
    // Number to be displayed
    String tmpString = "#ZL50,90,";
    tmpString += i;
    tmpString += "\r";
    sendData(tmpString, 12);
  } else {
    // Number to be displayed
    String tmpString = "#ZL28,04,";
    tmpString += i;
    tmpString += "\r";  
    sendData(tmpString, 12);
  }
}

void initScreen() {
  // Disable terminal mode
  sendData("#TA", 3);
  delay(2);
  
  // Clear screen
  sendData("#DL", 3);
  delay(2);
  if (VERTICAL_SCREEN) {
    // text orientation horizontal
    sendData("#ZW1,", 5);
    delay(2);
    // Font selection
    sendData("#ZF0,", 5);
    delay(2);
    // zoom factor 4
    sendData("#ZZ4,4,", 8);
    delay(2);
  } else {
    // text orientation horizontal
    sendData("#ZW0,", 5);
    delay(2);
    // Font selection
    sendData("#ZF7,", 5);
    delay(2);
    // zoom factor 1
    sendData("#ZZ1,1,", 8);
    delay(2);
  }
}

void sendData(String buf, unsigned char len) {
  unsigned char i, bcc;
  mySerial.write(0x11);
  bcc = 0x11;
  mySerial.write(len);
  bcc = bcc + len;
  for(i=0; i < len; i++) {
    mySerial.write(buf[i]);
    bcc = bcc + buf[i];
  }
  mySerial.write(bcc);
}
