#include <iostream>
#include <stdio.h>
#include <NmeaParser/NmeaParser.h>
#include <IreneTargetSpeed/IreneTargetSpeed.h>

using std::cout;

void printRow(const NmeaParser &p) {
    cout << p.year() << " " << p.month() << " " << p.day()
        << " " << p.hour() << " " << p.min() << " " << p.sec();
    cout << " " << p.gpsSpeedAsString();  // 7
    cout << " " << p.awaAsString();       // 8
    cout << " " << p.awsAsString();       // 9
    cout << " " << p.twaAsString();       // 10
    cout << " " << p.twsAsString();       // 11
    cout << " " << p.magHdgAsString();    // 12
    cout << " " << p.watSpeedAsString();  // 13
    cout << " " << p.gpsBearingAsString();// 14
    cout << " " << p.pos().lat.deg() << " " << p.pos().lat.min() << " " << p.pos().lat.mc();
    cout << " " << p.pos().lon.deg() << " " << p.pos().lon.min() << " " << p.pos().lon.mc();
    cout << " " << p.cwdAsString();
    cout << " " << p.wdAsString();
    cout << " " << getSpeedRatio(p.twa(), p.tws(), p.gpsSpeed());
    cout << "\n";
}

int main(int argc, char *argv[]) {
    int c;
    FILE *f = stdin;

    NmeaParser np;

    np.setIgnoreWrongChecksum(true);

    if (argc>=2) f = fopen(argv[1],"r");

    if (!f) {
        perror(argv[1]);
        return -1;
    }

    for(c=0; c!=EOF; c = fgetc(f)) {
        switch (np.processByte(c)) {
            case NmeaParser::NMEA_TIME_POS:
                printRow(np);
                break;
            default: break;
        }
    }

    fprintf(stderr, "Summary:\nBytes: %ld\nErrors: %d\n Sentences: %d\n",
           np.numBytes(),
           np.numErr(),
           np.numSentences());
    return 0;
}


