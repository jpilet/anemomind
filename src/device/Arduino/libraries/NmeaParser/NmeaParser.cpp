#include "nmeaparser.h"

#include <stdio.h>
#include <math.h>

namespace {

char HexDigitToInt(char data) {
    if( (data - '0') <= 9) {
        return (data - '0');
    } else {
        return (data - 'A' + 10);
    }
}

char intToHexDigit(char val) {
    val = val & 0xF;
    if (val < 10) {
        return val + '0';
    } else {
        return (val - 10) + 'A';
    }
}

#ifndef NOT_ON_MICROCONTROLLER
int strcmp(const char *str1, const char *str2) {
    while (*str1 && *str1 == *str2) {
        ++str1;
        ++str2;
    }
    return *str1;
}

int strlen(const char *str) {
    int l = 0;
    for (const char *s = str; *s; ++s) {
        ++l;
    }
    return l;
}
#endif

char parse2c(char *ab) {
	return (ab[0]-'0')*10 + ab[1]-'0';
}

Word parseNc(char *a, int n) {
    Word r=0;
    int i;
    for (i=0; i<n; i++) {
        if (a[i] >= '0' && a[i]<='9') {
            r = r*10 + (a[i]-'0');
        } else {
            break;
        }
    }
    for (int j = i; j < n; ++j) {
        r = r * 10;
    }
    return r;
}


DWord parseInt(char *s, int *n)
{
    DWord r=0;
    int i=0;
    if (n) *n=0;
    if (!s) return 0;
    while(s[i]>='0' && s[i]<='9') {
        r = r*10 + s[i]-'0';
        i++;
    }
    if (n) *n=i;
    return r;
}

DWord parseSpeed(char *speed, char *unit)
{
    int i,j;
    DWord frac;
    DWord r = parseInt(speed,&i) << 8;
    if (speed[i]=='.') {
        i++;
        frac = parseInt(speed+i, &j);
        frac <<=8;
        for (i=0;i<j;i++) {
            frac/=10;
        }
        r+=frac;
    }
    //assert(unit[0] == 'N');
    return r;
}

}  // namespace

NmeaParser::NmeaParser() {
    receivedChecksum_ = 0;
    numBytes_ = 0;
    numErr_ = 0;
    numSentences_ = 0;
    state_ = NP_STATE_SOM;
    ignoreWrongChecksum_ = false;
    argc_ = 0;
    data_[0] = 0;

    gpsSpeed_ = INVALID_DATA_SHORT;
    gpsBearing_ = INVALID_DATA_SHORT;
    hour_ = INVALID_DATA_CHAR;
    min_ = INVALID_DATA_CHAR;
    sec_ = INVALID_DATA_CHAR;
    day_ = month_ = year_ = INVALID_DATA_CHAR;

    awa_ = INVALID_DATA_SHORT;
    twa_ = INVALID_DATA_SHORT;
    aws_ = INVALID_DATA_SHORT;
    tws_ = INVALID_DATA_SHORT;
    magHdg_ = INVALID_DATA_SHORT;
    watSpeed_ = INVALID_DATA_SHORT;
    cwd_ = INVALID_DATA_LONG;
    wd_ = INVALID_DATA_LONG;
}

NmeaParser::NmeaSentence NmeaParser::processByte(Byte input) {
    NmeaSentence ret = NMEA_NONE;
    numBytes_++;

    switch(state_)
    {
        // Search for start of message '$'
        case NP_STATE_SOM :
            if(input == '$')
            {
                checksum_ = 0;		// reset checksum
                index_ = 0;		// reset index
                argc_ = 1;
                argv_[0] = data_;
                state_ = NP_STATE_CMD;
            }
            break;

        case NP_STATE_IMPL_EOS:
            // last character was a '$', both terminating a sentence and starting a new one.
            checksum_ = 0;			// reset checksum
            index_ = 0;				// reset index
            argc_ = 1;
            argv_[0] = data_;
            state_ = NP_STATE_CMD;

            // Retrieve command (NMEA Address)
        case NP_STATE_CMD :
            if (input == ',')
            {
                data_[index_++] = '\0';	// terminate command
                argv_[argc_++] = data_ + index_;
                checksum_ ^= input;
                state_ = NP_STATE_CMD;		// goto get data state_

                // Check for command overflow
                if(index_ >= NP_MAX_DATA_LEN)
                {
                    state_ = NP_STATE_SOM;
                    numErr_++;
                }
            } else if (input == '*') {
                data_[index_] = '\0';
                state_ = NP_STATE_CHECKSUM_1;

            } else if(input == '\r' || input == '\n') {

                // Check for end of sentence with no checksum
                //
                data_[index_] = '\0';
                state_ = NP_STATE_SOM;
                ret = processCommand();
            } else if (allow_implicit_eos_ && input == '$') {
                // implicit end of sentence
                data_[index_] = '\0';
                state_ = NP_STATE_IMPL_EOS;
                ret = processCommand();
            } else {
                data_[index_++] = input;
                checksum_ ^= input;

                // Check for command overflow
                if(index_ >= NP_MAX_DATA_LEN)
                {
                    state_ = NP_STATE_SOM;
                    numErr_++;
                }
            }
            break;

        case NP_STATE_CHECKSUM_1 :
            receivedChecksum_ = HexDigitToInt(input) << 4;
            state_ = NP_STATE_CHECKSUM_2;
            break;

        case NP_STATE_CHECKSUM_2 :
            receivedChecksum_ |= HexDigitToInt(input);

            if(ignoreWrongChecksum_ || checksum_ == receivedChecksum_)
            {
                ret = processCommand();
            } else {
                numErr_++;
            }
            state_ = NP_STATE_SOM;

            break;

        default: state_ = NP_STATE_SOM;
    }
    return ret;
}

char NmeaParser::computeChecksum() const {
    int i;
    char checksum = 0;
    for (char *s = argv_[0]; *s; ++s) {
        checksum ^= *s;
    }
    for (i=1;i< argc_;i++) {
        checksum ^= ',';
        for (char *s = argv_[i]; *s; ++s) {
            checksum ^= *s;
        }
    }
    return checksum;
}

void NmeaParser::getSentenceString(int bufLen, char *buffer) {
    int pos = 0;
    buffer[pos++] = '$';
    for (int i = 0; i < argc_; ++i) {
        if (i > 0) {
            buffer[pos++] = ',';
        }
        for (char *p = argv_[i]; *p; ++p) {
            buffer[pos++] = *p;
        }
    }
    buffer[pos++] = '*';
    char checksum = computeChecksum();
    buffer[pos++] = intToHexDigit(checksum >> 4);
    buffer[pos++] = intToHexDigit(checksum & 0xF);
    buffer[pos++] = '\n';
    buffer[pos++] = 0;
}

void NmeaParser::printSentence() {
    char buffer[82];
    getSentenceString(82, buffer);
    printf("%s", buffer);
}

void NmeaParser::setXTE(float dist, bool left) {
    argc_ = 1;
    argv_[0] = data_;
    int distInt = (int)(dist);
    sprintf(data_, "GPRMB,A,0.00,L,,PERF,4622.300,N,00619.430,E,%d.%03d,227.1,,V,A",
            distInt,
            (int)((dist - distInt) * 1000 + 0.5));
}

NmeaParser::NmeaSentence NmeaParser::processCommand() {
    numSentences_++;

    char *c = argv_[0] + 2;

    if (strcmp(c, "RMC") == 0) {
        return processGPRMC();
    } else if (strcmp(c, "MWV") == 0) {
        return processMWV();
    } else if (strcmp(c, "VHW") == 0) {
        return processVHW();
    } else if (strcmp(c, "VLW") == 0) {
        return processVLW();
    }

    return NMEA_UNKNOWN;
}

NmeaParser::NmeaSentence NmeaParser::processGPRMC() {
    if (argc_<10) return NMEA_NONE;
    if (strlen(argv_[1]) != 6) return NMEA_NONE;
    if (strlen(argv_[9]) != 6) return NMEA_NONE;

    gpsSpeed_ = parseSpeed(argv_[7], argv_[8]);
    hour_ = parse2c(argv_[1]);
    min_ = parse2c(argv_[1]+2);
    sec_ = parse2c(argv_[1]+4);
    day_ = parse2c(argv_[9]);
    month_ = parse2c(argv_[9]+2);
    year_ = parse2c(argv_[9]+4);

    pos_.lat.set(
        (argv_[4][0] == 'S' ? -1 : 1) * parse2c(argv_[3]),
        parse2c(argv_[3]+2),
        parseNc(argv_[3]+5,3));

    pos_.lon.set(
       (argv_[6][0] == 'W' ? -1 : 1) * parseNc(argv_[5],3),
       parse2c(argv_[5]+3),
       parseNc(argv_[5]+6,3));

    gpsBearing_ = parseInt(argv_[8],0);

    return NMEA_TIME_POS;
}

NmeaParser::NmeaSentence NmeaParser::processMWV() {
    if (argc_<=5 || argv_[5][0] != 'A') return NMEA_NONE;

    DWord s = parseSpeed(argv_[3], argv_[4]);

    if (argv_[2][0] == 'R') {
        aws_ = s;
        awa_ = parseInt(argv_[1],0);
        return NMEA_AW;
    }
    if (argv_[2][0] == 'T') {
        tws_ = s;
        twa_ = parseInt(argv_[1],0);
        return NMEA_TW;
    }
    return NMEA_NONE;
}

NmeaParser::NmeaSentence NmeaParser::processVHW() {
    if (argc_<6) return NMEA_NONE;

    magHdg_ = parseInt(argv_[3],0);
    watSpeed_ = parseSpeed(argv_[5], argv_[6]);

    return NMEA_WAT_SP_HDG;
}

// $IIVLW,00430,N,002.3,N*55
NmeaParser::NmeaSentence NmeaParser::processVLW() {
    int n=0;

    if (argc_!=5) return NMEA_NONE;

    cwd_ = parseInt(argv_[1], 0);
    wd_ = 10*parseInt(argv_[3], &n);
    wd_ += parseInt(argv_[3]+n+1,0);
    return NMEA_VLW;
}


double AccAngle::toDouble() const {
    double subdeg = ((double)min_ + (double)mc_/1000.0) / 60.0;
    return (double)deg_ + (deg_ < 0 ? -subdeg : subdeg);
}

AccAngle::AccAngle() {
    deg_ = NmeaParser::INVALID_DATA_SHORT;
    min_ = NmeaParser::INVALID_DATA_SHORT;
    mc_ = NmeaParser::INVALID_DATA_SHORT;
}

void AccAngle::set(double dbl) {
    char sign = (dbl < 0 ? -1 : 1);
    double angleAbs = (dbl < 0 ? -dbl : dbl);
    int degrees = (int)(angleAbs);
    double minutes = (angleAbs - degrees) * 60;
    deg_ = sign * degrees;
    min_ = (int)(minutes);
    mc_ = (int)((minutes - min_) * 1000 + .5);
}


static void wgs84ToXYZ(double lonDeg, double latDeg, double altitude,
                       double *xyz, double *dlon, double *dlat)
{
    const double k2_PI = 6.283185307179586476925286766559005768394338798750211641949889184615;
    const double kDeg2Grad = (k2_PI/360.0);
    const double a = 6378137; // semi-major axis of ellipsoid
    const double f = 1.0/298.257223563; // flatening of ellipsoid
    const double latRad = latDeg * kDeg2Grad;
    const double lonRad = lonDeg * kDeg2Grad;
    const double sinlat = sin(latRad);
    const double coslat = cos(latRad);
    const double sinlon = sin(lonRad);
    const double coslon = cos(lonRad);
    const double e2 =  f*(2-f); //  eccentricity^2
    double t3,t4,t5,t6,t8,t9,t11,t13,t16,t17,t18,t19,t23,t26,t31,t36,t37;

    /* mapple code:
        with(linalg);
        v := a/sqrt(1-e^2*sin(lat)^2);
        X := (v+altitude)*cos(lat)*cos(lon);
        Y := (v+altitude)*cos(lat)*sin(lon);
        Z := (v*(1-e^2)+h)*sin(lat);
        J := jacobian([X,Y,Z], [lon,lat]);
        llon := sqrt((J[1,1]^2 + J[2,1]^2 + J[3,1]^2));
        llat := sqrt((J[1,2]^2 + J[2,2]^2 + J[3,2]^2));
        r := vector([X,Y,Z,llon,llat]);
     */

    t3 = sinlat*sinlat;
    t4 = e2*t3;
    t5 = 1.0-t4;
    t6 = sqrt(t5);
    t8 = a/t6;
    t9 = t8+altitude;
    t11 = t9*coslat;
    t13 = t11*sinlon;
    t16 = a/t6/t5;
    t17 = t16*e2;
    t18 = coslat*coslat;
    t19 = sinlat*t18;
    t23 = t9*sinlat;
    t26 = t11*coslon;
    t31 = 1.0-e2;
    t36 = t8*t31+altitude;
    t37 = t17*t19;

    if (dlon) {
        *dlon = sqrt(t13*t13 + t26*t26)*kDeg2Grad;
    }

    if (dlat) {
        double u = (t37*coslon-t23*coslon);
        double v = (t37*sinlon-t23*sinlon);
        double w = (t16*t31*t4*coslat+t36*coslat);
        *dlat = sqrt(u*u + v*v + w*w)*kDeg2Grad;
    }

    if (xyz) {
        xyz[0] = t26;
        xyz[1] = t13;
        xyz[2] = t36*sinlat;
    }
}

ProjectedPos::ProjectedPos(const GeoRef &ref, const GeoPos &pos) {
    ref.project(pos, xy_);
}

double ProjectedPos::dist(const ProjectedPos &p) const {
    double dx = x() - p.x();
    double dy = y() - p.y();
    return sqrt(dx * dx + dy * dy);
}

void GeoRef::set(const GeoPos &pos, double altitude) {
    reflon = pos.lon.toDouble();
    reflat = pos.lat.toDouble();
    wgs84ToXYZ(reflon, reflat, altitude, 0, &dlon, &dlat);
}

void GeoRef::project(const GeoPos &pos, double xy[2]) const {
    xy[0] = dlon * (pos.lon.toDouble() - reflon);
    xy[1] = dlat * (pos.lat.toDouble() - reflat);
}

#ifdef NOT_ON_MICROCONTROLLER
#include <sstream>
using std::string;
using std::stringstream;

namespace {

template <typename T>
string toString(T a) {
    stringstream r;
    r << a;
    return r.str();
}

}  // namespace

string NmeaParser::awaAsString() const {
    return toString(awa());
}

string NmeaParser::gpsSpeedAsString() const {
    return toString(gpsSpeed() / 256.0);
}

string NmeaParser::awsAsString() const {
    return toString(aws() / 256.0);
}

string NmeaParser::twaAsString() const {
    return toString(twa());
}

string NmeaParser::twsAsString() const { 
    return toString(tws() / 256.0);
}

string NmeaParser::magHdgAsString() const {
    return toString(magHdg());
}

string NmeaParser::watSpeedAsString() const {
    return toString(watSpeed() / 256.0);
}

string NmeaParser::gpsBearingAsString() const {
    return toString(gpsBearing());
}

string NmeaParser::cwdAsString() const {
    return toString(cwd());
}

string NmeaParser::wdAsString() const {
    return toString(wd() / 10.0);
}

#endif
