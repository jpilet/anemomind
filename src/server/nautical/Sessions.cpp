/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/Sessions.h>
#include <server/math/PiecewisePolynomials.h>
#include <server/plot/extra.h>

namespace sail {
namespace Sessions {


Array<Array<Nav> > coarseSplit(Array<Nav> navs, Duration<double> dur) {
  int from = 0;
  ArrayBuilder<Array<Nav> > dst;
  for (int i = 0; i < navs.size()-1; i++) {
    int to = i+1;
    if (navs[to].time() - navs[i].time() > dur) {
      dst.add(navs.slice(from, to));
      from = to;
    }
  }
  dst.add(navs.sliceFrom(from));
  return dst.get();
}

void fineSplit(Array<Nav> navs, Duration<double> maxRms, ArrayBuilder<Session> *dst) {
  auto first = navs.first().time();
  int n = navs.size();
  Arrayd X(n);
  Arrayd Y(n);
  for (int i = 0; i < n; i++) {
    X[i] = i;
    Y[i] = (navs[i].time() - first).seconds();
  }


  double maxCost = n*sqr(maxRms.seconds());
  auto pieces = PiecewisePolynomials::optimizeForMaxCost<2>(X, Y, n, LineKM::identity(), maxCost);
  for (auto piece: pieces) {
    int from = piece.span.minv();
    int to = piece.span.maxv();
    auto navSlice = navs.slice(from, to);
    auto session = Session{navSlice,
          Duration<double>::seconds(piece.line().getK()),
          Duration<double>::seconds(piece.line().getM()),
          Duration<double>::seconds(sqrt(piece.cost()/navSlice.size()))};
    if (session.rms > Duration<double>::seconds(100)) {
      GnuplotExtra plot;
      plot.plot_xy(X, Y);
      plot.plot_xy(X.slice(from, to), Y.slice(from, to));
      plot.set_xlabel("Sample index");
      plot.set_ylabel("Time (seconds)");
      plot.show();
    }
    dst->add(session);
  }
}

Array<Session> segment(Array<Nav> navs, Duration<double> maxRms) {
  std::sort(navs.begin(), navs.end());
  auto splitted = coarseSplit(navs, Duration<double>::days(1.0));
  ArrayBuilder<Session> dst;
  for (auto x: splitted) {
    fineSplit(x, maxRms, &dst);
  }
  return dst.get();
}

std::ostream &operator<<(std::ostream &dst, const Session &x) {
  dst << "Session of " << x.navs.size() << " navs from " << x.navs.first().time()
      << " to " << x.navs.last().time() << " sampled with a period of " << x.averageSamplingPeriod.seconds() << " seconds and rms of " << x.rms.seconds() << " seconds." << std::endl;
  return dst;
}

}
}
