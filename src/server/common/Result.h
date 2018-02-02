/*
 * Result.h
 *
 *  Created on: 2 Feb 2018
 *      Author: jonas
 */

#ifndef SERVER_COMMON_RESULT_H_
#define SERVER_COMMON_RESULT_H_

#include <string>

namespace sail {

struct Result {
  bool success = true;
  std::string explanation;

  static Result success() {return Result();}

  static Result failure(const std::string& s) {
    Result r;
    r.success = false;
    r.explanation = s;
    return r;
  }
};

}

#endif /* SERVER_COMMON_RESULT_H_ */
