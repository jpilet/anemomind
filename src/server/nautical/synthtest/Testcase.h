/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TESTCASE_H_
#define TESTCASE_H_

namespace sail {

/*
 * A Testcase holds testdata for a single race that can
 * contain any number of boats. More specific, it holds the following:
 *
 * Common to all boats:
 *   * Local wind conditions in space and time
 *   * Local current conditions in space and time
 *   * A geographic reference point
 *   * A time offset
 *
 * Per boat:
 *   *
 *
 */
class Testcase {
 public:
  typedef std::function<HorizontalMotion<double>(Length<double> x, Length<double> y,
      Duration<double>)> FlowFun;


  Testcase();

  void addBoat(BoatSpecific);

  // Test data specific to a single boat
  class BoatData {
   public:
  };
 private:

};

} /* namespace mmm */

#endif /* TESTCASE_H_ */
