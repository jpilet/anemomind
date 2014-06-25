
#define ANEMOMIND_DEVICE
#include <PhysicalQuantity.h>

#include <NmeaParser.h>
#include <TargetSpeed.h>
#include <SD.h>
#include <ChunkFile.h>
#include <SoftwareSerial.h>

#include <InstrumentFilter.h>

using namespace sail;

const bool VERTICAL_SCREEN = false;

char logFilePath[13];

File logFile;
NmeaParser nmeaParser;
unsigned long lastFlush = 0;
bool echo = false;
int flushFrequMs = 10000;

TargetSpeedTable targetSpeedTable;
InstrumentFilter<FP16_16> filter;

SoftwareSerial mySerial(8, 9); // RX, TX

int my_putc(char c, FILE *) {
  if (echo) {
    Serial.write( c );
  }
  if (logFile) {
    // We assume buffered output.
    logFile.write(c);
  }
}

void openLogFile() {
  for (int i = 0; i < 9999; ++i) {
    sprintf(logFilePath, "nmea%04d.txt", i);
    if (!SD.exists(logFilePath)) {
      logFile = SD.open(logFilePath, O_WRITE | O_CREAT);
      break;
    }
  }
}

#undef degrees

void displaySpeedRatio(const NmeaParser& parser) {
   float speedRatio = getVmgSpeedRatio(targetSpeedTable,
       parser.twa().degrees(),
       parser.tws().knots(),
       filter.gpsSpeed().knots());
   
   // Display speedRatio on the LCD display.
   updateScreen(max(0,min(200, int(speedRatio * 100.0))));
}

void loadData() {
  ChunkTarget targets[] = {
    makeChunkTarget(&targetSpeedTable)
  };
  
  ChunkLoader loader(targets, sizeof(targets) / sizeof(targets[0]));
  
  {
    File dataFile = SD.open("boat.dat");

    if (dataFile) {
      while (dataFile.available()) {
        loader.addByte(dataFile.read());
      }
      dataFile.close();
    }
  }

  if (!targets[0].success) {
    invalidateSpeedTable(&targetSpeedTable);
    updateScreen(-2);
  } else {
    updateScreen(-1);
  }
}

void sendScreenData(String buf) {
  unsigned char i, bcc;
  const int len = buf.length();
  mySerial.write(0x11);
  bcc = 0x11;
  mySerial.write(len);
  bcc = bcc + len;
  for(i=0; i < len; i++) {
    mySerial.write(buf[i]);
    bcc = bcc + buf[i];
  }
  mySerial.write(bcc);
  delay(2);
}

void initScreen() {
  delay(3);

  // Disable terminal mode
  sendScreenData("#TA,");
  
  // Clear screen
  sendScreenData("#DL,");
  
  // Turn backlight off
  sendScreenData("#YL0,");
  
  if (VERTICAL_SCREEN) {
    // text orientation horizontal
    sendScreenData("#ZW1,");
    // Font selection
    sendScreenData("#ZF0,");
    // zoom factor 4
    sendScreenData("#ZZ4,4,");
  } else {
    // text orientation horizontal
    sendScreenData("#ZW0,");
    // Font selection
    sendScreenData("#ZF7,");
    // zoom factor 1
    sendScreenData("#ZZ1,1,");
  }
}

void updateScreen(int i) {
  char str[16];
  sprintf(str,
          (VERTICAL_SCREEN ? "#ZL50,90,%02d\r" : "#ZC0,04,%03d\r"),
          i);
  sendScreenData(str);
}

void setup()
{
  mySerial.begin(115200);
  initScreen();
  updateScreen(1);
  delay(1000);
  updateScreen(2);

  // Open serial communications and wait for port to open:
  Serial.begin(4800);
  updateScreen(3);

  fdevopen( &my_putc, 0);
 
  updateScreen(4);

  // SD Card initialization.
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work.
  pinMode(10, OUTPUT);
  updateScreen(5);

  if (SD.begin(10)) {
  }
  updateScreen(5);

  openLogFile();
  updateScreen(6);

  loadData();
}

void loop()
{
  while (Serial.available() > 0) {
    char c = Serial.read();
    Serial.write(c);
    
    switch (nmeaParser.processByte(c)) {
      case NmeaParser::NMEA_NONE: break;
      case NmeaParser::NMEA_WAT_SP_HDG:
        filter.setMagHdg(nmeaParser.magHdg());
        filter.setWatSpeed(nmeaParser.watSpeed());
        break;
      case NmeaParser::NMEA_AW:
        filter.setAwa(nmeaParser.awa());
        filter.setAws(nmeaParser.aws());
        break;
      case NmeaParser::NMEA_TIME_POS:
        filter.setGpsSpeed(nmeaParser.gpsSpeed());
        filter.setGpsBearing(nmeaParser.gpsBearing());

        displaySpeedRatio(nmeaParser);      
      default:
        logNmeaSentence();
        break;
    }
  }

  if (logFile) {
    // Flush the write buffer to the flash drive every couple of seconds.
    unsigned long now = millis();
    // detect overflow
    if (now < lastFlush) {
      lastFlush = now;
    }
    if ((now - lastFlush) > flushFrequMs) {
      logFile.flush();
      lastFlush = now;
    }
  }
}

void logNmeaSentence() {
  nmeaParser.printSentence();
}



