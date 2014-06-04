#include <NmeaParser.h>
#include <TargetSpeed.h>
#include <SD.h>
#include <ChunkFile.h>

char logFilePath[13];

File logFile;
NmeaParser nmeaParser;
unsigned long lastFlush = 0;
bool echo = false;
int flushFrequMs = 3000;

TargetSpeedTable targetSpeedTable;

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

void sendData(const NmeaParser& parser) {
   float speedRatio = getVmgSpeedRatio(targetSpeedTable,
       parser.twa(),
       FP8_8(parser.tws()) / FP8_8(256),
       FP8_8(parser.gpsSpeed()) / FP8_8(256));
   // TODO: display speedRatio on the LCD display.
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
  }  
}

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(4800);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  fdevopen( &my_putc, 0);
 
  // SD Card initialization.
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work.
  pinMode(10, OUTPUT);

  if (SD.begin(10)) {
  }

  openLogFile();

  loadData();
}

void loop()
{
  while (Serial.available() > 0) {
    char c = Serial.read();
    Serial.write(c);
    
    switch (nmeaParser.processByte(c)) {
      case NmeaParser::NMEA_NONE: break;
      case NmeaParser::NMEA_TIME_POS:
        logNmeaSentence();
        break;
      default: logNmeaSentence(); break;
    }
  }

  if (logFile) {
    // Flush the write buffer to the flash drive every couple of seconds.
    unsigned long now = millis();
    if ((now - lastFlush) > flushFrequMs) {
      sendData(nmeaParser);
      logFile.flush();
      lastFlush = now;
    }
  }
}

void logNmeaSentence() {
  nmeaParser.printSentence();
}



