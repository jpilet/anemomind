/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/FractalFlow.h>

using namespace sail;

namespace {
  void windFlowDemo001() {
    auto flow = makeWindFlow001();
  }
}

int main(int argc, const char **argv) {
  windFlowDemo001();
  return 0;
}
