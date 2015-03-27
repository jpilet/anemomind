
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
#include <avr/wdt.h>
#endif

#include <InstrumentFilter.h>
#include <PolarSpeedTable.h>

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

#define VMG_TARGET_SPEED

#ifdef VMG_TARGET_SPEED
TargetSpeedTable targetSpeedTable;
#else
PolarSpeedTable polarSpeedTable;
#endif

InstrumentFilter<FP16_16, ::TimeStamp, Duration<long> > filter;

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
  Angle<FP16_16> twdir, twa;
  Velocity<FP8_8> tws;

  if (calibrationLoaded) {
    HorizontalMotion<FP16_16> wind =
      TrueWindEstimator::computeTrueWind(calibration.params, filter);

    twdir = calcTwdir(wind);
    tws = wind.norm();
    // Todo: compute TWA with TrueWindEstimator.
    twa = (twdir - Angle<FP16_16>(parser.magHdg()));
  } else {
    twdir = (Angle<FP16_16>(parser.twa()) + Angle<FP16_16>(parser.magHdg())).positiveMinAngle();
    tws = parser.tws();
    twa = parser.twa();
  }
  
  float speedRatio;
  #ifdef VMG_TARGET_SPEED
  speedRatio = getVmgSpeedRatio(targetSpeedTable,
       twa.degrees(),
       tws.knots(),
       (FP8_8) (filter.gpsSpeed().knots()));
  #else
  Velocity<FP16_16> targetSpeed = polarSpeedTable.targetSpeed(tws, twa);
  if (targetSpeed > Velocity<FP16_16>::knots(.5) &&
      filter.gpsSpeed() > Velocity<FP16_16>::knots(.5)) {
    speedRatio = float(targetSpeed / filter.gpsSpeed());    
  } else {
    speedRatio = 0;
  }
  #endif
 
   // Display speedRatio on the LCD display.
   screenUpdate(
     max(0,min(200, int(speedRatio * 100.0))),
     twdir.degrees(),
     tws.knots()
     );
}

void loadData() {
  ChunkTarget targets[] = {
    makeChunkTarget(&calibration)
#ifdef VMG_TARGET_SPEED    
    ,makeChunkTarget(&targetSpeedTable)
#endif
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

  calibrationLoaded = targets[0].success;
  
#ifdef VMG_TARGET_SPEED
  if (!targets[0].success) {
    invalidateSpeedTable(&targetSpeedTable);
  }
#else
  polarSpeedTable.load("polar.dat");
#endif

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
  
#ifdef ARDUINO_UNO
  // reset after 2 seconds, if no "pat the dog" received
  wdt_enable(WDTO_2S);
#endif
}

void logNmeaSentence() {
  nmeaParser.putSentence(my_putc);
}

void loop()
{
#ifdef ARDUINO_UNO
  if (millis() < (unsigned long)(1000 * 60 * 60 * 24)) {
    wdt_reset();  // Tell the watchdog everything is OK.
  } else {
    // Reset after 24 hours.
    logFile.flush();
    wdt_enable(WDTO_15MS);
    delay(20);
  }
#endif

  while (Serial.available() > 0) {
    char c = Serial.read();
    
    ::TimeStamp timestamp = ::TimeStamp::now();
    switch (nmeaParser.processByte(c)) {
      case NmeaParser::NMEA_NONE: break;
      case NmeaParser::NMEA_WAT_SP_HDG:
        filter.setMagHdgWatSpeed(nmeaParser.magHdg().cast<FP16_16>(),
                                 nmeaParser.watSpeed().cast<FP16_16>(),
                                 timestamp);
        logNmeaSentence();
        break;
      case NmeaParser::NMEA_AW:
        filter.setAw(nmeaParser.awa().cast<FP16_16>(),
                     nmeaParser.aws().cast<FP16_16>(),
                     timestamp);
        logNmeaSentence();
        break;
      case NmeaParser::NMEA_TIME_POS:
        filter.setGps(nmeaParser.gpsBearing().cast<FP16_16>(),
                      nmeaParser.gpsSpeed().cast<FP16_16>(),
                      timestamp);

        displaySpeedRatio(nmeaParser);      
        logNmeaSentence();
        break;
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
