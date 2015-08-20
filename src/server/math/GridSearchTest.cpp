/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/ArrayIO.h>
#include <iostream>
#include <server/common/string.h>
#include <gtest/gtest.h>
#include <server/math/GridSearch.h>


using namespace sail;
using namespace sail::GridSearch;

TEST(GridSearchTest, Test0) {

  int maxStep = 9;
  auto x = makeGrid<2>(maxStep);
  EXPECT_EQ(x.rows(), 1 + 2*maxStep);
  EXPECT_TRUE(std::isinf(x(0, 0)));

  Arrayi pos{10, 10};
  Arrayi initX{90, 93};
  Arrayi Xgt{91, 94};
  Arrayi X = pos2X<2>(initX, pos, maxStep);
  EXPECT_EQ(X, Xgt);
}

TEST(GridSearchTest, Minimize) {
  Objf objf = [](Arrayi X) {
    double a = X[0] - 5;
    double b = X[1] - 6;
    return a*a + b*b;
  };

  Settings s;
  Arrayi Xinit{0, 0};
  Arrayi Xopt = minimize<2>(objf, Xinit, s);
  Arrayi Xgt{5, 6};
  EXPECT_EQ(Xopt, Xgt);
}



