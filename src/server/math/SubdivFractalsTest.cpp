/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/SubdivFractals.h>
#include <gtest/gtest.h>
#include <server/common/Array.h>

using namespace sail;
using namespace sail::SubdivFractals;

TEST(SubdivFractalsTest, IndexBox) {
  EXPECT_EQ(IndexBox<2>().numel(), 0);
  IndexBox<2> rulle = IndexBox<1>(2) + IndexBox<1>(3);
  EXPECT_EQ(rulle.numel(), 2*3);
  auto a = rulle.slice(0, 1);
  auto b = rulle.slice(1, 1);
  EXPECT_EQ(a.numel(), 3);
  EXPECT_EQ(b.numel(), 2);
  EXPECT_EQ(rulle.slice(1, 0, 2).numel(), 2*2);
  int inds[2] = {1, 1};
  EXPECT_EQ(rulle.calcIndex(inds), 1 + 1*2);
  EXPECT_FALSE(rulle.hasMidpoint());
}

TEST(SubdivFractalsTest, IndexBoxMidpoint) {
  EXPECT_EQ(IndexBox<2>().numel(), 0);
  IndexBox<2> rulle = IndexBox<1>(3) + IndexBox<1>(5);
  EXPECT_TRUE(rulle.hasMidpoint());
  EXPECT_EQ(rulle.midpointIndex(), 7);
  EXPECT_EQ(rulle.lowIndex(), 0);
  EXPECT_EQ(rulle.highIndex(), 14);
}

TEST(SubdivFractalsTest, IndexList) {
  IndexList<3> indexList;
  EXPECT_EQ(indexList.size(), 3);
  for (int i = 0; i < 3; i++) {
    EXPECT_EQ(indexList[i], i);
  }
  IndexList<3> reduced = indexList.remove(1);
  EXPECT_EQ(reduced.size(), 2);
  EXPECT_EQ(reduced[0], 0);
  EXPECT_EQ(reduced[1], 2);
}

TEST(SubdivFractalsTest, MaxSlope) {
  MaxSlope maxSlope(1.0, 1.0);
  EXPECT_NEAR(maxSlope.eval(0.5), 1.0, 1.0e-6);
  EXPECT_NEAR(maxSlope.eval(0.9), 1.0, 1.0e-6);
  EXPECT_NEAR(maxSlope.eval(2.0), 0.5, 1.0e-6);

  EXPECT_NEAR(maxSlope.fitValue(0.0, 0.0, 0.5, -0.5, 0.1), 0.05, 1.0e-6);
  EXPECT_NEAR(maxSlope.fitValue(0.0, 0.0, 0.5,  0.5, 0.1), 0.0,  1.0e-6);
}



//TEST(SubdivFractalsTest, Generate) {
//  MDArray2i indexTable(7, 7);
//  indexTable.setAll(1);
//
//  {
//    const int ruleCount = 3;
//    int A[ruleCount] = {0, 0, 2};
//    int B[ruleCount] = {3, 2, 3};
//    int C[ruleCount] = {4, 5, 6};
//    for (int i = 0; i < ruleCount; i++) {
//      indexTable(A[i], B[i]) = C[i];
//    }
//  }
//
//  IndexBox<2> box = IndexBox<1>(3) + IndexBox<1>(3);
//
//  int vertexTypes[9] = {0, -1, 2,  -1, -1, -1,  2, -1, 3};
//  int expectedVertexTypes[9] = {0, 5, 2, 5, 4, 6, 2, 6, 3};
//
//  MDArray2d lambda(7, 7);
//  lambda.setAll(0.5);
//
//  double vertices[9] = {0, NAN, 1,  NAN, NAN, NAN,  1, NAN, 2};
//  double expectedVertices[9] = {0, 0.5, 1, 0.5, 1, 1.5, 1, 1.5, 2};
//
//  box.generate<double>(vertexTypes, vertices, indexTable, lambda);
//  for (int i = 0; i < 9; i++) {
//    EXPECT_EQ(vertexTypes[i], expectedVertexTypes[i]);
//    EXPECT_NEAR(vertices[i], expectedVertices[i], 1.0e-9);
//  }
//}
//
//TEST(SubdivFractalsTest, BaseCase) {
//  MDArray2i inds(1, 1);
//  inds.setAll(0);
//  MDArray2d lambda(1, 1);
//  lambda.setAll(0.5);
////  SubdivFractals<2> fractal(inds, lambda);
////  double ctrl[4] = {0, 2, 2, 3};
////  int classes[4] = {0, 0, 0, 0};
////  {
////    double coords[2] = {0, 0};
////    EXPECT_NEAR(fractal.eval(coords, ctrl, classes, 0), 0.0, 1.0e-6);
////  }{
////    double coords[2] = {1, 1};
////    EXPECT_NEAR(fractal.eval(coords, ctrl, classes, 0), 3.0, 1.0e-6);
////  }{
////    double coords[2] = {1, 0};
////    EXPECT_NEAR(fractal.eval(coords, ctrl, classes, 0), 2.0, 1.0e-6);
////  }{
////    double coords[2] = {0.5, 0.5};
////    EXPECT_NEAR(fractal.eval(coords, ctrl, classes, 0), 7.0/4, 1.0e-6);
////  }{
////    double coords[2] = {0.5, 1};
////    EXPECT_NEAR(fractal.eval(coords, ctrl, classes, 0), 2.5, 1.0e-6);
////  }
//
//}
