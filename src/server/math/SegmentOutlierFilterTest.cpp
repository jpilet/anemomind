/*
 * SegmentOutlierFilterTest.cpp
 *
 *  Created on: 4 Aug 2017
 *      Author: jonas
 */

#include <server/math/SegmentOutlierFilter.h>
#include <gtest/gtest.h>

using namespace sail;
using namespace sof;

typedef std::array<double, 1> Vec1;

Vec1 v1(double x) {
  return std::array<double, 1>{x};
}

TEST(SegmentOutlierFilter, BasicTest) {
  double h[5] = {4, 40, 6, 7.33, 1};
  auto a = Segment<1>::primitive(0, 3, v1(h[0]));
  auto b = Segment<1>::primitive(1, 4, v1(h[1]));
  auto c = Segment<1>::primitive(2, 5, v1(h[2]));
  auto d = Segment<1>::primitive(3, 6, v1(h[3]));
  auto e = Segment<1>::primitive(4, 7, v1(h[4]));

  EXPECT_NEAR(a.cost(), 0.0, 1.0e-6);
  EXPECT_NEAR(b.cost(), 0.0, 1.0e-6);
  EXPECT_NEAR(c.cost(), 0.0, 1.0e-6);

  double w2 = 1.0;
  double w1 = 0.0;

  auto ab = join(a, b, w2, w1);
  auto de = join(d, e, w2, w1);
  EXPECT_EQ(ab.paramCount, 2);

  EXPECT_NEAR(ab.cost(), 0.0, 1.0e-6);
  EXPECT_NEAR(de.cost(), 0.0, 1.0e-6);

  auto abc = join(ab, c, w2, w1);
  auto abc2 = join(a, join(b, c, w2, w1), w2, w1);

  EXPECT_EQ(3, abc.paramCount);
  EXPECT_LT(1, abc.cost());
  EXPECT_NEAR(abc.cost(), abc2.cost(), 1.0e-6);

  auto abcde = join(abc, de, w2, w1);
  EXPECT_LT(abc.cost(), abcde.cost());

  auto cd = join(c, d, w2, w1);
  auto abcd = join(ab, cd, w2, w1);





  EXPECT_EQ(abcde.inds[0], 0);
  EXPECT_EQ(abcde.inds[1], 1);
  EXPECT_EQ(abcde.inds[2], 3);
  EXPECT_EQ(abcde.inds[3], 4);

  EXPECT_EQ(abcde.X[0], 3);
  EXPECT_EQ(abcde.X[1], 4);
  EXPECT_EQ(abcde.X[2], 6);
  EXPECT_EQ(abcde.X[3], 7);

  int n = 5;
  int rd = n - 2;
  Eigen::MatrixXd A = Eigen::MatrixXd::Zero(n + rd, n);
  Eigen::VectorXd B = Eigen::VectorXd::Zero(n + rd);
  for (int i = 0; i < n; i++) {
    A(i, i) = 1.0;
    B(i) = h[i];
  }
  double reg[3] = {0.5, -1, 0.5};
  for (int i = 0; i < rd; i++) {
    int row = n + i;
    int col = i;
    for (int j = 0; j < 3; j++) {
      A(row, col + j) = reg[j]*w2;
    }
  }
  Eigen::VectorXd X = A.colPivHouseholderQr().solve(B);
  auto expectedCost = (A*X - B).squaredNorm();

  auto actual = abcde;
  CHECK(actual.paramCount == std::min(4, n));

  EXPECT_NEAR(actual.cost(), expectedCost, 1.0e-6);
}




void initializePoints(
    double x, double step, double f,
    int l, int u,
    Array<std::pair<double, Vec1>>* dst) {
  if (l + 1 == u) {
     (*dst)[l] = {l, {x}};
  } else if (l < u) {
    int middle = (l + u)/2;
    initializePoints(x - step, step*f, f, l, middle, dst);
    initializePoints(x + step, step*f, f, middle, u, dst);
  }
}


TEST(SegmentOutlierFilterTest, Optimize) {
  Settings s;
  s.verbose = true;
  Array<std::pair<double, Vec1>> points(16);
  initializePoints(0, 1, 0.2, 0, 16, &points);
  auto mask = optimize<1>(points, s);
}
