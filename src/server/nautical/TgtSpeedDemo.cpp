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

  std::string filename = "/home/jonas/programmering/matlab/irene_tgt_speed/allnavs.txt";

  MDArray<double, 2> data = loadMatrixText<double>(filename);
  assert(!data.empty());
  return 0;
}


