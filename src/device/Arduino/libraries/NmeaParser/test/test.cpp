#include "../NmeaParser.h"

#include <string.h>
#include <stdio.h>
#include <math.h>

#define CHECK_EQ(a, b) checkEq(a, b, #a, #b)
#define CHECK_NEAR(a, b, delta) checkNear(a, b, delta, #a, #b, #delta)

double fabs(double a) {
  return (a< 0 ? -a : a);
}

bool success = true;
const char *currentTest = "";
void checkEq(int a, int b, const char *stra, const char *strb) {
  if (a != b) {
    success = false;
    printf("%s (%d) != %s (%d)\n", stra, a, strb, b);
  }
}
void checkNear(double a, double b, double delta,
               const char *stra, const char *strb, const char *strdelta) {
  if (fabs(b - a) > delta) {
    success = false;
    printf("|%s (%f) - %s (%f)| > %s(%f)\n",
           stra, a, strb, b, strdelta, delta);
  }
}

void startTest(const char *s) {
  success = true;
  currentTest = s;
}

bool endTest() {
  printf("Test: %s: %s\n", currentTest, (success ? "passed" : "failed"));
  return success;
}

void sendSentence(const char *sentence, NmeaParser *parser) {
  for (const char *s = sentence; *s; ++s) {
    parser->processByte(*s);
  }
  char buffer[82];
  parser->getSentenceString(82, buffer);
  for (int i = 0; sentence[i]; ++i) {
    CHECK_EQ(buffer[i], sentence[i]);
  }
}

bool TestVLW() {
  startTest("VLW");

  NmeaParser parser;

  sendSentence("$IIVLW,00430,N,002.3,N*55", &parser);

  CHECK_EQ(430, parser.cumulativeWaterDistance());
  CHECK_EQ(23, parser.waterDistance());
  CHECK_EQ(1, parser.numSentences());
  return endTest();
}

bool TestRMC() {
  startTest("RMC");

  NmeaParser parser;

  sendSentence(
    "$IIRMC,130222,A,4612.929,N,00610.063,E,01.5,286,100708,,,A*4E",
    &parser);

  CHECK_EQ(46, parser.pos().lat.deg());
  CHECK_EQ(12, parser.pos().lat.min());
  CHECK_EQ(929, parser.pos().lat.mc());
  CHECK_EQ(6, parser.pos().lon.deg());
  CHECK_EQ(10, parser.pos().lon.min());
  CHECK_EQ(63, parser.pos().lon.mc());

  CHECK_EQ(1, parser.numSentences());

  sendSentence(
    "$GPRMC,081836,A,3751.65,S,14507.36,E,000.0,360.0,130998,011.3,E*62",
    &parser);

  CHECK_EQ(-37, parser.pos().lat.deg());
  CHECK_EQ(51, parser.pos().lat.min());
  CHECK_EQ(650, parser.pos().lat.mc());
  CHECK_EQ(145, parser.pos().lon.deg());
  CHECK_EQ(7, parser.pos().lon.min());
  CHECK_EQ(360, parser.pos().lon.mc());
  CHECK_EQ(2, parser.numSentences());

  sendSentence(
    "$GPRMC,225446,A,4916.45,N,12311.12,W,000.5,054.7,191194,020.3,E*68",
    &parser);

  CHECK_EQ(49, parser.pos().lat.deg());
  CHECK_EQ(16, parser.pos().lat.min());
  CHECK_EQ(450, parser.pos().lat.mc());
  CHECK_EQ(-123, parser.pos().lon.deg());
  CHECK_EQ(11, parser.pos().lon.min());
  CHECK_EQ(120, parser.pos().lon.mc());
  CHECK_EQ(3, parser.numSentences());

  return endTest();
}

bool TestAccAngle() {
  startTest("AccAngle");

  AccAngle angle(3.2);
  CHECK_NEAR(angle.toDouble(), 3.2, 1e-6);

  double asDegrees = -302.2865;
  angle = AccAngle(asDegrees);
  CHECK_NEAR(angle.toDouble(), asDegrees, 1e-6);

  asDegrees = -300.12345;
  angle = AccAngle(asDegrees);
  CHECK_NEAR(angle.toDouble(), asDegrees, 1e-6);

  angle.set(-45, 30, 123);

  asDegrees = - (45.0 + (30.0 + (123.0 / 1000.0)) / 60.0);
  CHECK_NEAR(angle.toDouble(), asDegrees, 1e-6);

  return endTest();
}

bool TestPrintXTE() {
  startTest("PrintXTE");

  NmeaParser parser;

  parser.setXTE(14.619, false);
  char buffer[82];
  parser.getSentenceString(82, buffer);

  parser.printSentence();

  NmeaParser read;
  for (char *s = buffer; *s; ++s) {
    read.processByte(*s);
  }
  CHECK_EQ(1, read.numSentences());
  CHECK_EQ(0, read.numErr());

  return endTest();
}

bool TestGPRMB() {
  startTest("GPRMB");
  NmeaParser parser;
  const char *sentence = "$GPRMB,V,,,,YVOIRE,4622.300,N,00619.430,E,,,,V,N*29\n";
  sendSentence(sentence, &parser);
  char buffer[82];
  parser.getSentenceString(82, buffer);
  CHECK_EQ(0, strcmp(sentence, buffer));

  return endTest();
}

void testDist(const GeoPos &a, const GeoPos &b,
              double altitude, double dist) {
  GeoRef ref(a, altitude);
  ProjectedPos pa(ref, a);
  ProjectedPos pb(ref, b);
  CHECK_NEAR(dist, pa.dist(pb), 1);
}

bool TestGeoRef() {
  startTest("TestGeoRef");

  // The geographic coordinates and distances come from Google Earth. This
  // test verifies that distances computed match the ones computed by Google
  // Earth.

  GeoPos markA;
  markA.lat.set(54, 50, 422);
  markA.lon.set(9, 29, 885);

  GeoPos markB;
  markB.lat.set(54, 52, 58);
  markB.lon.set(9, 33, 726);

  GeoRef ref(markA, 0);

  ProjectedPos projA(ref, markA);

  CHECK_NEAR(0, projA.x(), .01);
  CHECK_NEAR(0, projA.y(), .01);

  ProjectedPos projB(ref, markB);

  CHECK_NEAR(5109.1, projA.dist(projB), 3);

  testDist(
    GeoPos(46, 29, 624, 6, 36, 658),
    GeoPos(46, 30, 98, 6, 39, 677),
    372,
    3962.5);

  testDist(
    GeoPos(-42, 6, 31, -61, 35, 886),
    GeoPos(-42, 6, 957, -61, 35, 854),
    0,
    1715);

  return endTest();
}

int main() {
  bool returnValue = true;

  if (!TestVLW()) returnValue = false;
  if (!TestRMC()) returnValue = false;
  if (!TestAccAngle()) returnValue = false;
  if (!TestPrintXTE()) returnValue = false;
  if (!TestGeoRef()) returnValue = false;
  if (!TestGPRMB()) returnValue = false;

  return (returnValue ? 0 : 1);
}
