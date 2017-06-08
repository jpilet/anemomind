/*
 * RawNmea2000Logger.h
 *
 *  Created on: 8 Jun 2017
 *      Author: jonas
 */

#ifndef DEVICE_ANEMOBOX_LOGGER_RAWNMEA2000LOGGER_H_
#define DEVICE_ANEMOBOX_LOGGER_RAWNMEA2000LOGGER_H_

namespace sail {

struct RawNmea2000Sentence {
  int64_t timestamp_us;
  std::string data;
};

class RawNmea2000Logger {
public:
  RawNmea2000Logger();
  virtual ~RawNmea2000Logger();
private:
  //std::map<int64_t, std::vector<>>
};

} /* namespace sail */

#endif /* DEVICE_ANEMOBOX_LOGGER_RAWNMEA2000LOGGER_H_ */
