#include <Screen.h>
//#include <SoftwareSerial.h>

/*
 DIP128W-6LW
 Example Code
 
 xavier@thefabric.com
 */

void setup() {
  Serial.begin(57600);
  screenInit();
  Serial.println("DIP128W-6LW Example Code running...");
}

void loop() {
  for (int i=0; i<200; i+=5) {
    // Display some random numbers.
    screenUpdate(i, (i * 3 + 537) % 128, (i * 2 + 833) % 128);
    delay(300); 
  }
}


