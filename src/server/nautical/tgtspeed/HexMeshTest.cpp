/*
 * HexMeshTest.cpp
 *
 *  Created on: 16 Mar 2018
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/tgtspeed/HexMesh.h>

using namespace sail;

TEST(HexMeshTest, BasicTest) {
  HexMesh mesh(2, 1.0);
}
