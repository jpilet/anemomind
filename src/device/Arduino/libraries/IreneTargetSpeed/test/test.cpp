#include "../IreneTargetSpeed.h"

#include <stdio.h>

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

bool TestInterpolate() {
    startTest("Interpolate");

    const float counts[3] = { 0, 1, 2 };
    const float epsilon = .0001f;

    CHECK_NEAR(0, interpolate(counts, sizeof(counts) / sizeof(counts[0]), 0), epsilon);
    CHECK_NEAR(1, interpolate(counts, sizeof(counts) / sizeof(counts[0]), 1), epsilon);
    CHECK_NEAR(2, interpolate(counts, sizeof(counts) / sizeof(counts[0]), 2), epsilon);
    CHECK_NEAR(2, interpolate(counts, sizeof(counts) / sizeof(counts[0]), 3), epsilon);
    CHECK_NEAR(0, interpolate(counts, sizeof(counts) / sizeof(counts[0]), -1), epsilon);
    CHECK_NEAR(1.32, interpolate(counts, sizeof(counts) / sizeof(counts[0]), 1.32), epsilon);

    const float doubles[] = { 0, 2, 4, 6, 8 };
    
    for (float f = 0; f < 4; f += .1) {
	CHECK_NEAR( f * 2, interpolate(doubles, sizeof(doubles) / sizeof(doubles[0]), f), epsilon);
    }

    return endTest();
}

bool TestUpWind() {
    startTest("UpWind");
    CHECK_NEAR(1.0, getSpeedRatio(45, 10, 5.5), .2);
    return endTest();
}

bool TestDownWind() {
    startTest("DownWind");
    CHECK_NEAR(1.0, getSpeedRatio(160, 10, 5.8), .2);
    return endTest();
}

int main() {
    TestInterpolate();
    TestUpWind();
    TestDownWind();
    return 0;
}
