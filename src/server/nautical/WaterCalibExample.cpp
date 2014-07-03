/*
 *  Created on: 2014-07-03
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/nautical/WaterCalib.h>

using namespace sail;

int main(int argc, const char **argv) {
  Array<Nav> navs = getTestdataNavs().sliceTo(5000);
  return 0;
}


