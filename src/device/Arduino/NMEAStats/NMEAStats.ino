
#define ANEMOMIND_DEVICE
#include <PhysicalQuantity.h>
#include <TrueWindEstimator.h>

#include <NmeaParser.h>
#include <TargetSpeed.h>
#include <SD.h>
#include <ChunkFile.h>

#include <Screen.h>
#ifdef ARDUINO_UNO
#include <SoftwareSerial.h>
#include <SPI.h>
#endif

#include <InstrumentFilter.h>

using namespace sail;

class TimeStamp {
  public:
  TimeStamp() : _timeMilliseconds(0) { }
  bool undefined() const { return _timeMilliseconds == 0; }
  
  Duration<long> operator - (TimeStamp other) const {
    return Duration<long>::milliseconds(_timeMilliseconds - other._timeMilliseconds);
  }
  
  static TimeStamp now() {
    TimeStamp result;
    result._timeMilliseconds = millis();
    return result;
  }
  
  private:
    unsigned long _timeMilliseconds;
};

namespace {
const bool echo = false;
const int flushFrequMs = 10000;

char logFilePath[13];


File logFile;
NmeaParser nmeaParser;
unsigned long lastFlush = 0;

TargetSpeedTable targetSpeedTable;
InstrumentFilter<FP16_16, TimeStamp, Duration<long> > filter;

TrueWindEstimator::Parameters<FP16_16> calibration;
bool calibrationLoaded = false;

}  // namespace

void my_putc(char c) {
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
  Angle<FP8_8> twa;
  Velocity<FP8_8> tws;

  if (calibrationLoaded) {
    HorizontalMotion<FP16_16> wind =
      TrueWindEstimator::computeTrueWind(calibration.params, filter);

    twa = wind.angle();
    tws = wind.norm();
  } else {
    twa = parser.twa();
    tws = parser.tws();
  }

   float speedRatio = getVmgSpeedRatio(targetSpeedTable,
       twa.degrees(),
       tws.knots(),
       filter.gpsSpeed().knots());
   
   // Display speedRatio on the LCD display.
   screenUpdate(
     max(0,min(200, int(speedRatio * 100.0))),
     twa.degrees(),
     tws.knots()
     );
}

void loadData() {
  ChunkTarget targets[] = {
    makeChunkTarget(&targetSpeedTable),
    makeChunkTarget(&calibration)
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

  calibrationLoaded = targets[1].success;
  
  if (!targets[0].success) {
    invalidateSpeedTable(&targetSpeedTable);
  }
  
  screenUpdate((targets[0].success ? 0 : -1) + (targets[1].success? 0 : -2));
}


void setup()
{
  screenInit();
  screenUpdate(1);

  // Open serial communications and wait for port to open:
  Serial.begin(4800);
  screenUpdate(2);

  // SD Card initialization.
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work.
  pinMode(10, OUTPUT);
  screenUpdate(3);

  SD.begin(10);
  
  screenUpdate(4);

  openLogFile();
  screenUpdate(5);

  loadData();
}

void loop()
{
  while (Serial.available() > 0) {
    char c = Serial.read();
    Serial.write(c);
    
    TimeStamp timestamp = TimeStamp::now();
    switch (nmeaParser.processByte(c)) {
      case NmeaParser::NMEA_NONE: break;
      case NmeaParser::NMEA_WAT_SP_HDG:
        filter.setMagHdgWatSpeed(nmeaParser.magHdg().cast<FP16_16>(),
                                 nmeaParser.watSpeed().cast<FP16_16>(),
                                 timestamp);
        break;
      case NmeaParser::NMEA_AW:
        filter.setAw(nmeaParser.awa().cast<FP16_16>(),
                     nmeaParser.aws().cast<FP16_16>(),
                     timestamp);
        break;
      case NmeaParser::NMEA_TIME_POS:
        filter.setGps(nmeaParser.gpsBearing().cast<FP16_16>(),
                      nmeaParser.gpsSpeed().cast<FP16_16>(),
                      timestamp);

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
  nmeaParser.putSentence(my_putc);
}



