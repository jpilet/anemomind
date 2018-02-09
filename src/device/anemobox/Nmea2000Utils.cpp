/*
 * BasicNMEA2000.cpp
 *
 *  Created on: 26 Jan 2018
 *      Author: jonas
 */

#include <device/anemobox/Nmea2000Utils.h>
#include <chrono>
#include <thread>

extern "C" {

  // The NMEA2000 library of ttlappalainen links to these
  // functions, but it is up to us to define them for the
  // platform.
  //
  // TODO: Maybe put these functions somewhere else than in
  // Nmea2000Utils in order to avoid linkage problems, in case
  // we want to use some other implementation than this one.
  uint32_t millis() {
    using namespace std;
    using namespace std::chrono;
    static auto start = system_clock::now().time_since_epoch();
    auto now = system_clock::now().time_since_epoch();
    milliseconds ms = duration_cast< milliseconds >(now - start);
    return ms.count();
  }
  void delay(uint32_t t) {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(t));
  }

}
