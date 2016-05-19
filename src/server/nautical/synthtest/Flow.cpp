/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Flow.h"
#include <server/common/Array.h>
#include <server/common/LineKM.h>
#include <server/plot/extra.h>
#include <server/common/string.h>
#include <armadillo>
#include <server/common/logging.h>

namespace sail {

namespace {
  Velocity<double> eval(const Flow::VelocityFunction &fun,
      const Flow::ProjectedPosition &p, Duration<double> t) {
    if (fun) {
      return fun(p, t);
    }
    return Velocity<double>::knots(0.0);
  }

  Flow::VelocityFunction operator+ (const Flow::VelocityFunction &a, const Flow::VelocityFunction &b) {
    if (!bool(a)) {
      return b;
    } else if (!bool(b)) {
      return a;
    }

    return [=](const Flow::ProjectedPosition &p, Duration<double> t) {
      return a(p, t) + b(p, t);
    };
  }
}

Flow::Flow(VelocityFunction x, VelocityFunction y) {
  _funs[0] = x;
  _funs[1] = y;
}

Flow::VelocityFunction Flow::spatiallyChangingVelocity(
    Velocity<double> amplitude,
    Angle<double> angle, Length<double> period, Angle<double> phase) {
    return [=](const ProjectedPosition &p, Duration<double> t) {
      Length<double> proj = cos(angle)*p[0] + sin(angle)*p[1];
      double at = (proj/period).getScalar();
      double f = sin(2.0*M_PI*at + phase.radians());
      return f*amplitude;
    };
}


Flow Flow::constant(const HorizontalMotion<double> &m) {
  return constant(m[0], m[1]);
}

Flow Flow::constant(Velocity<double> x, Velocity<double> y) {
  return Flow([=](const ProjectedPosition &p, Duration<double> t) {
                  return x;
              },[=](const ProjectedPosition &p, Duration<double> t) {
                  return y;
              });
}


HorizontalMotion<double> Flow::operator() (const ProjectedPosition &p, Duration<double> t) const {
  return HorizontalMotion<double>(eval(_funs[0], p, t), eval(_funs[1], p, t));
}

Flow Flow::operator+ (const Flow &other) const {
  return Flow(_funs[0] + other._funs[0], _funs[1] + other._funs[1]);
}

std::function<HorizontalMotion<double>(Flow::ProjectedPosition, Duration<double>)> Flow::asFunction() const {

  // So that we copy the data, and not the pointer.
  Flow thisData = *this;

  return [=](const ProjectedPosition &p, Duration<double> t) {
    return thisData(p, t);
  };
}

void Flow::plot1d(ProjectedPosition fromPos, Duration<double> fromTime,
    ProjectedPosition   toPos, Duration<double>   toTime,
    GnuplotExtra *dst) const {

  int sampleCount = 1000;
  Array<double> X(sampleCount);
  Array<double> Y[2] = {Array<double>(sampleCount), Array<double>(sampleCount)};
  LineKM lambdaMap(0, sampleCount-1, 0.0, 1.0);
  Velocity<double> maxvel = Velocity<double>::zero();
  for (int i = 0; i < sampleCount; i++) {
    double lambda = lambdaMap(i);
    X[i] = lambda;
    ProjectedPosition pos = (1.0 - lambda)*fromPos + lambda*toPos;
    Duration<double> time = (1.0 - lambda)*fromTime + lambda*toTime;
    double y[2];
    for (int j = 0; j < 2; j++) {
      y[j] = _funs[j](pos, time).knots();
      Y[j][i] = y[j];
    }
    maxvel = std::max(maxvel, Velocity<double>::knots(sqrt(sqr(y[0]) + sqr(y[1]))));
  }
  dst->plot_xy(X, Y[0]);
  dst->plot_xy(X, Y[1]);
  std::cout << "Max velocity (m/s):   " << maxvel.metersPerSecond() << std::endl;
  std::cout << "Max velocity (knots): " << maxvel.knots() << std::endl;
}

void Flow::plotForPosition(ProjectedPosition at,
      Duration<double> fromTime, Duration<double> toTime) const {
  GnuplotExtra plot;
  plot.set_style("lines");
  plot.set_title(stringFormat("Plot dim %d for position (%.3g, %.3g) nautical miles for %.3g hours",
      at[0].nauticalMiles(), at[1].nauticalMiles(), (toTime - fromTime).hours()));
  plot1d(at, fromTime, at, toTime, &plot);
  plot.show();
}

namespace {
  Spani calcIndexSpan(Span<Length<double> > lspan, Length<double> spacing) {
    return Spani(int(floor((lspan.minv()/spacing).getScalar())),
        int(ceil((lspan.maxv()/spacing).getScalar())));
  }

  MDArray2d makeArrow(const arma::vec2 &src,
                      const arma::vec2 &dst) {
    const arma::vec2 &offset = src;
    arma::vec2 xaxis = dst - src;
    arma::vec2 yaxis = arma::vec2{xaxis[1], -xaxis[0]};
    arma::vec2 pts[5] = {0.0*xaxis,
                          1.0*xaxis,
                          0.5*xaxis - 0.25*yaxis,
                          1.0*xaxis,
                          0.5*xaxis + 0.25*yaxis};
    MDArray2d data(5, 2);
    for (int i = 0; i < 5; i++) {
      for (int j = 0; j < 2; j++) {
        data(i, j) = offset[j] + pts[i][j];
      }
    }
    return data;
  }
}

void Flow::plotVectorField(Duration<double> time, BBox<Length<double>, 2> area,
    Length<double> spacing) const {

  Spani xspan = calcIndexSpan(area.getSpan(0), spacing);
  Spani yspan = calcIndexSpan(area.getSpan(1), spacing);

  Velocity<double> maxvel = Velocity<double>::zero();
  for (auto xi: xspan) {
    for (auto yi: yspan) {
      ProjectedPosition pos = ProjectedPosition{double(xi)*spacing,
                      double(yi)*spacing};
      HorizontalMotion<double> motion = (*this)(pos, time);
      Velocity<double> vel = motion.norm();
      maxvel = std::max(maxvel, vel);
    }
  }
  std::cout << "Max grid velocity (m/s):   " << maxvel.metersPerSecond() << std::endl;
  std::cout << "Max grid velocity (knots): " << maxvel.knots() << std::endl;

  GnuplotExtra plot;
  plot.set_style("lines");
  double f = 0.5*spacing.nauticalMiles();
  for (auto xi: xspan) {
    for (auto yi: yspan) {
      ProjectedPosition pos = ProjectedPosition{double(xi)*spacing,
                      double(yi)*spacing};
      HorizontalMotion<double> motion = (*this)(pos, time);
      arma::vec2 gridpos{pos[0].nauticalMiles(), pos[1].nauticalMiles()};
      arma::vec2 griddir{f*(motion[0]/maxvel).getScalar(),
        f*(motion[1]/maxvel).getScalar()};
      plot.plot(makeArrow(gridpos - griddir, gridpos + griddir));
    }
  }
  plot.show();
}



} /* namespace mmm */
