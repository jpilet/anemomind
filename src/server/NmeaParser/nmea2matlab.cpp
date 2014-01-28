#include <stdio.h>
#include <NmeaParser/NmeaParser.h>
#include <IreneTargetSpeed/IreneTargetSpeed.h>

void printRow(const NmeaParser &p) {
    printf("%d %d %d %d %d %d", p.year(), p.month(), p.day(), p.hour(), p.min(), p.sec());
    printf(" %f", p.gpsSpeed() / 256.0f); 	// 7
    printf(" %d", p.awa()); 			// 8
    printf(" %f", p.aws()/256.0f);		// 9
    printf(" %d", p.twa());			// 10
    printf(" %f", p.tws()/256.0f);		// 11
    printf(" %d", p.magHdg());	// 12
    printf(" %f", p.watSpeed()/256.0f);	// 13
    printf(" %d", p.gpsBearing());	// 14
    printf(" %d %d %d", p.pos().lat.deg(), p.pos().lat.min(), p.pos().lat.mc());
    printf(" %d %d %d", p.pos().lon.deg(), p.pos().lon.min(), p.pos().lon.mc());
    printf(" %ld", p.cwd());
    printf(" %f", p.wd()/10.0f);
    printf(" %f", getSpeedRatio(p.twa(), p.tws(), p.gpsSpeed()));
    printf("\n");
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


