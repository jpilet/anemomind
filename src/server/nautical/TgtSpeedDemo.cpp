/*
 *  Created on: 2014-08-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/Nav.h>
#include <server/common/ArrayIO.h>

int main(int argc, const char **argv) {
  using namespace sail;

  Poco::Path p = PathBuilder::makeDirectory("/home/jonas/programmering/matlab/irene_tgt_speed/allnavs.txt").get();

  MDArray<double, 2> data = loadMatrixText<double>(p.toString());
  assert(!data.empty());
  return 0;
}


