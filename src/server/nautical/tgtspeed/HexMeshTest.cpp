/*
 * HexMeshTest.cpp
 *
 *  Created on: 16 Mar 2018
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/tgtspeed/HexMesh.h>
#include <server/transducers/Transducer.h>

using namespace sail;

TEST(HexMeshTest, BasicTest1) {
  HexMesh mesh(1, 2.0);
  EXPECT_EQ(12, mesh.edges().size());
  mesh.dispVertexLayout();
}


TEST(HexMeshTest, BasicTest) {
  HexMesh mesh(3, 2.0);

  {
    auto xw = Eigen::Vector2d(3, 4);
    auto xg = mesh.worldToGrid(xw);
    EXPECT_LT(0.1, (xw - xg).norm());
    EXPECT_NEAR((mesh.gridToWorld(mesh.worldToGrid(xw)) - xw).norm(),
        0.0, 1.0e-6);
  }

  {
    auto x = Eigen::Vector2d(0.0, 0.0);
    auto y = mesh.worldToGrid(x);
    EXPECT_LT(0.1, y(0));
    EXPECT_LT(0.1, y(1));
  }

  {
    auto x = Eigen::Vector2d(0.0, 0.0);
    auto y = mesh.worldToGrid(x);
    EXPECT_LT(0.1, y(0));
    EXPECT_LT(0.1, y(1));
    EXPECT_NEAR(y(0), 3.0, 0.01);
    EXPECT_NEAR(y(1), 3.0, 0.01);
  }

  {
    auto x = Eigen::Vector2d(1.0, 0.0);
    auto y = mesh.worldToGrid(x);
    EXPECT_LT(0.1, y(0));
    EXPECT_LT(0.1, y(1));
    EXPECT_NEAR(y(0), 3.5, 0.01);
    EXPECT_NEAR(y(1), 3.0, 0.01);
  }

  {
    auto x = Eigen::Vector2d(0.0, 0.0);
    auto y = mesh.gridToWorld(x);
    EXPECT_LT(y(0), -0.1);
    EXPECT_LT(y(1), -0.1);
  }

  mesh.dispVertexLayout();

  EXPECT_EQ(37, mesh.vertexCount());

  EXPECT_FALSE(mesh.represent(Eigen::Vector2d(1000, 1000)).defined());
  {
    auto r = Eigen::Vector2d(0.5, 0.3);
    auto wi0 = mesh.represent(r);
    EXPECT_TRUE(wi0.defined());
    auto wi = wi0.get();
    EXPECT_FALSE(
        transduce(wi,
            trMap([](WeightedIndex wi) {return wi.index == -1;}),
            IntoOr()));

    EXPECT_NEAR((r - mesh.eval(wi)).norm(), 0.0, 1.0e-6);
  }
}
