/*
 * Replay a NMEA0183 log at real speed, assuming GPS time is available 
 * in the log file.
 *
 * To use on an anemobox, simply do
 *
 *     playnmea log.txt > /dev/ttyMFD1
 */
#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>

using namespace sail;

namespace {

void playNmea(FILE *f) {
  NmeaParser parser;
  TimeStamp startReal = TimeStamp::now();
  TimeStamp startFake;
  bool hasFakeStartTime = false;

  while (1) {
    int c = fgetc(f);
    if (c== EOF) {
      break;
    }
    switch (parser.processByte(c)) {
      case NmeaParser::NMEA_NONE: break;
      case NmeaParser::NMEA_TIME_POS:
        if (hasFakeStartTime) {
          Duration<> elapsedReal = TimeStamp::now() - startReal;
          Duration<> elapsedFake = parser.timestamp() - startFake;
          if (elapsedReal < elapsedFake) {
            sleep(elapsedFake - elapsedReal);
          }
        } else {
          startFake = parser.timestamp();
          hasFakeStartTime = true;
        }
        // no break, execution continues at 'default'
      default:
        // We do not output anything before knowing the fake time.
        if (hasFakeStartTime) {
          parser.printSentence();
        }
        break;
    }
  }
}

}  // namespace

int main(int argc, char **argv) {

  if (argc < 1) {
    fprintf(stderr, "usage: %s <nmea log> [<nmea log>] ...\n", argv[0]);
    return 1;
  }

  for (int i = 1; i < argc; ++i) {
    FILE *f = fopen(argv[i], "r");
    if (!f) {
      perror(argv[i]);
      return 2;
    }

    playNmea(f);
    fclose(f);
  }

  return 0;
}
