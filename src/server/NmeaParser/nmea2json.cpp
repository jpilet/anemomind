#include <stdio.h>
#include <time.h>
#include <string.h>
#include <NmeaParser/NmeaParser.h>
#include <IreneTargetSpeed/IreneTargetSpeed.h>

time_t getTime(const NmeaParser &p) {
    struct tm t;
    memset(&t, 0, sizeof(t));
    t.tm_year = 100 + p.year();
    t.tm_mon = p.month();
    t.tm_mday = p.day();
    t.tm_hour = p.hour();
    t.tm_min = p.min();
    t.tm_sec = p.sec();
    return timegm(&t);
}

void printRow(const GeoRef &ref, const NmeaParser &p) {
    printf("{");
    printf("\"time\":%ld,", getTime(p));
    printf("\"gpsSpeed\":%f,", p.gpsSpeed() / 256.0f);
    printf("\"awa\":%d,", p.awa());
    printf("\"aws\":%f,", p.aws()/256.0f);
    printf("\"twa\":%d,", p.twa());
    printf("\"tws\":%f,", p.tws()/256.0f);
    printf("\"magHdg\":%d,", p.magHdg());
    printf("\"watSpeed\":%f,", p.watSpeed()/256.0f);	// 13
    printf("\"gpsBearing\":%d,", p.gpsBearing());	// 14
    printf("\"lat\":%f,", p.pos().lat.toDouble());
    printf("\"lon\":%f,", p.pos().lon.toDouble());
    printf("\"cwd\":%ld,", p.cwd());
    printf("\"wd\":%f,", p.wd()/10.0f);
    printf("\"speedRatio\":%f,", getSpeedRatio(p.twa(), p.tws(), p.gpsSpeed()));

    ProjectedPos pos(ref, p.pos());
    printf("\"x\":%f,", pos.x());
    printf("\"y\":%f", pos.y());
    printf("}\n");
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

    printf("[\n");
    GeoRef ref;
    bool hasRef = false;
    bool started = false;
    for(c=0; c!=EOF; c = fgetc(f)) {
        if (np.processByte(c) == NmeaParser::NMEA_TIME_POS) {
	    if (!hasRef) {
		ref.set(np.pos(), 0);
		hasRef = true;
	    }
            if (started) {
                printf(",");
            }
            started = true;
	    printRow(ref, np);
        }
    }
    printf("]\n");

    fprintf(stderr, "Summary:\nBytes: %ld\nErrors: %d\n Sentences: %d\n",
           np.numBytes(),
           np.numErr(),
           np.numSentences());
    return 0;
}


