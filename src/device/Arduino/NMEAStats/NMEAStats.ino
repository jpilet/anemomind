#include <NmeaParser.h>
#include <IreneTargetSpeed.h>
#include <SD.h>

char logFilePath[13];

File logFile;
NmeaParser nmeaParser;
unsigned long lastFlush = 0;
bool echo = false;
int flushFrequMs = 3000;

int my_putc( char c, FILE *t) {
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
   echo = true;
   float speedRatio = getSpeedRatio(parser.twa(), parser.tws(), parser.gpsSpeed());

   nmeaParser.setXTE(speedRatio, false);
   logNmeaSentence();
   echo = false;
   
   /*
   char* data =
  "$GPRMC,191122,A,4630.0884,N,00641.0583,E,0.0,240.8,210813,0.2,E,A*1E\n"
"$GPRMB,A,0.00,R,,YVOIRE,4622.300,N,00619.430,E,16.866,242.5,,V,A*43\n"
"$GPGGA,191122,4630.0884,N,00641.0583,E,1,06,2.2,378.0,M,48.2,M,,*46\n"
"$GPGSA,A,3,01,03,,11,14,,,,22,32,,,3.3,2.2,1.8*3D\n"
"$GPGSV,3,1,10,01,66,307,47,03,14,164,43,06,02,154,00,11,88,178,45*75\n"
"$GPGSV,3,2,10,14,37,065,46,17,09,320,00,19,42,163,43,20,35,240,38*73\n"
"$GPGSV,3,3,10,22,11,058,39,32,68,227,50*72\n"
"$GPGLL,4630.0884,N,00641.0583,E,191122,A,A*47\n"
"$GPBOD,242.5,T,242.3,M,YVOIRE,*5F\n"
"$PGRME,8.0,M,9.8,M,12.7,M*13\n"
"$PGRMZ,1064,f*37\n"
"$HCHDG,,,,0.2,E*05\n"
"$GPRTE,1,1,c,*37\n";
  
 Serial.write(data);
 */
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



