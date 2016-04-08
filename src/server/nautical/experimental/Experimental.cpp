/*
 * Experimental.cpp
 *
 *  Created on: Apr 7, 2016
 *      Author: jonas
 */

#include <server/nautical/experimental/Experimental.h>
#include <server/common/Functional.h>
#include <server/common/numerics.h>
#include <server/plot/extra.h>


namespace sail {
namespace Experimental {


void plotFlowsXY(Array<Eigen::Vector2d> flows) {
  int n = flows.size();

  MDArray2d A(n, 2);
  for (int i = 0; i < n; i++) {
    auto f = flows[i];
    A(i, 0) = f(0);
    A(i, 1) = f(1);
  }

  GnuplotExtra plot;
  plot.set_style("lines");
  plot.plot(A);
  plot.show();
}

void plotFlowsTime(Array<Eigen::Vector2d> flows) {
  int n = flows.size();

  MDArray2d X(n, 2), Y(n, 2);
  for (int i = 0; i < n; i++) {
    auto f = flows[i];
    X(i, 0) = i;
    X(i, 1) = f(0);
    X(i, 0) = i;
    Y(i, 1) = f(1);
  }

  GnuplotExtra plot;
  plot.set_style("lines");
  plot.plot(X);
  plot.plot(Y);
  plot.show();
}

void plotFlowsItg(Array<Eigen::Vector2d> flows) {
  int n = flows.size();

  MDArray2d X(n, 2);
  X(0, 0) = 0;
  X(0, 1) = 0;
  for (int i = 1; i < n; i++) {
    auto f = flows[i];
    X(i, 0) = f(0) + X(i-1, 0);
    X(i, 1) = f(1) + X(i-1, 1);
  }

  GnuplotExtra plot;
  plot.set_style("lines");
  plot.plot(X);
  plot.show();
}

Eigen::Vector2d getGpsVector(const Nav &x) {
  auto motion = x.gpsMotion();
  return Eigen::Vector2d(motion[0].knots(), motion[1].knots());
}

Eigen::Vector2d getBoatThroughWaterMotion(const Nav &x) {
  auto m = HorizontalMotion<double>::polar(x.watSpeed(), x.magHdg());
  return Eigen::Vector2d(m[0].knots(), m[1].knots());
}

bool isFiniteMat(const Eigen::MatrixXd &X) {
  for (int i = 0; i < X.rows(); i++) {
    for (int j = 0; j < X.cols(); j++) {
      if (!isFinite(X(i, j))) {
        return false;
      }
    }
  }
  return true;
}

FlowMats makeCurrentMatrix(const Nav &x) {
  Eigen::Vector2d a = -getBoatThroughWaterMotion(x);

  auto aHat = a.normalized();

  Eigen::Vector2d b = getGpsVector(x);
  FlowMats dst;

  dst.A(0, 0) = a(0);
  dst.A(0, 1) = -a(1);
  dst.A(1, 0) = a(1);
  dst.A(1, 1) = a(0);

  dst.A(0, 2) = aHat(0);
  dst.A(0, 3) = -aHat(1);
  dst.A(1, 2) = aHat(1);
  dst.A(1, 3) = aHat(0);

  dst.B(0) = b(0);
  dst.B(1) = b(1);

  return dst;
}

Eigen::Vector4d getDefaultParams() {
  Eigen::Vector4d X = Eigen::Vector4d::Zero();
  X(0) = 1.0;
  return X;
}

bool isFiniteFlowMats(const FlowMats &x) {
  return isFiniteMat(x.A) && isFiniteMat(x.B);
}

Array<FlowMats> makeCurrentMats(const Array<Nav> &navs) {
  return sail::filter(sail::map(navs, &makeCurrentMatrix), &isFiniteFlowMats);
}

}
}
