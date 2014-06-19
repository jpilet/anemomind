/*
 *  Created on: 2014-06-19
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef USERHINT_H_
#define USERHINT_H_

#include <server/common/TimeStamp.h>

namespace sail {

class UserHint {
 public:
  enum HintType {
    UNDEFINED = 0, RACE_START, RACE_END, NUM_TYPES
  };

  UserHint() : _type(UNDEFINED) {}
  UserHint(HintType type_, TimeStamp time_) : _type(type_), _time(time_) {}

  HintType type() const {
    return _type;
  }

  TimeStamp time() const {
    return _time;
  }

  UserHint(std::string type, TimeStamp time);
  std::string typeAsString() const;
 private:
  HintType _type;
  TimeStamp _time;
};

} /* namespace sail */

#endif /* USERHINT_H_ */
