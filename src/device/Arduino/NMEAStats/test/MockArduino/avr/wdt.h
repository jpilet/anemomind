#ifndef DEVICE_TEST_AVR_WDT_H
#define DEVICE_TEST_AVR_WDT_H

enum {
  WDTO_2S = 2000,
  WDTO_15MS = 15,
};

static inline void wdt_enable(int) { }
static inline void wdt_reset() { }

#endif
