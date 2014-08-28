/*
 *  Created on: 2014-08-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/common/PathBuilder.h>

int main(int argc, const char *argv) {
  using namespace sail;

  Array<Nav> data =  PathBuilder::makeDirectory("/home/jonas/programmering/matlab/irene_tgt_speed/allnavs.txt").get();
  assert(!data.empty());

  return 0;
}


