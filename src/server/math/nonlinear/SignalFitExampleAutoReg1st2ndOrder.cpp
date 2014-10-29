/*
 *  Created on: 2014-02-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "SignalFitExamples.h"

using namespace sail;

int main() {
  std::default_random_engine engine(0);
  sfitexAutoReg1st2ndOrder(engine);
}
