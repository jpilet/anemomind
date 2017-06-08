/*
 * RawNmea2000Logger.cpp
 *
 *  Created on: 8 Jun 2017
 *      Author: jonas
 */

#include "RawNmea2000Logger.h"

namespace sail {

RawNmea2000Logger::RawNmea2000Logger() {
  // TODO Auto-generated constructor stub

}

void RawNmea2000Logger::add(
    int64_t id, TimeStamp time, const std::string& data) {
  auto f = _sentenceChannels.find(id);
}

} /* namespace sail */
