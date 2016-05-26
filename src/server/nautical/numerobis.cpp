/*
 * numerobis.cpp
 *
 *  Created on: May 26, 2016
 *      Author: jonas
 */

#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/GeographicReference.h>
#include <server/plot/extra.h>
#include <server/common/Span.h>
#include <server/common/PhysicalQuantityIO.h>
#include <server/common/ArrayBuilder.h>

using namespace sail;


template <typename T>
Array<Span<sail::TimeStamp> > listSpans(
    const TimedSampleRange<T> &r) {
  ArrayBuilder<Span<sail::TimeStamp> > spans;
  TimeStamp start = r[0].time;
  TimeStamp last = r[0].time;
  for (int i = 1; i < r.size(); i++) {
    auto t = r[i].time;
    if (t - last > Duration<double>::minutes(1.0)) {
      spans.add(Span<TimeStamp>(start, last));
      start = t;
    }
    last = t;
  }
  return spans.get();
}

void showTimeSpans(const Array<Span<TimeStamp> > &spans) {
  for (int i = 0; i < spans.size(); i++) {
    auto span = spans[i];
    std::cout << "Span from " << span.minv() << " to " << span.maxv() << std::endl;
    if (i < spans.size() - 1) {
      auto next = spans[i + 1];
      std::cout << "\n   Gap of " << next.minv() - span.maxv() << "\n\n";
    }
  }
}

MDArray2d getPointsToPlot(const NavDataset &ds) {
  auto gps = ds.samples<GPS_POS>();

  std::cout << "GPS speed time stamps" << std::endl;
  showTimeSpans(listSpans(ds.samples<GPS_SPEED>()));
  std::cout << "GPS pos time stamps" << std::endl;
  showTimeSpans(listSpans(ds.samples<GPS_POS>()));
  std::cout << "Showed it" << std::endl;

  int n = gps.size();

  auto mid = gps[n/2];

  GeographicReference geoRef(mid.value);
  MDArray2d pts(n, 2);
  for (int i = 0; i < n; i++) {
    auto xy = geoRef.map(gps[i].value);
    pts(i, 0) = xy[0].meters();
    pts(i, 1) = xy[1].meters();
  }
  return pts;
}


void plotTheTrajectory(const NavDataset &ds) {
  GnuplotExtra plot;
  plot.set_style("lines");
  plot.plot(getPointsToPlot(ds));
  plot.show();
}

int main() {
  std::string dir = "/home/jonas/prog/anemomind/datasets/boat573b14bbf437a48c0ce3ad54";

  LogLoader loader;
  loader.load(dir);

  auto ds = loader.makeNavDataset().fitBounds();

  std::cout << "Loaded logs from " << ds.lowerBound() << " to " << ds.upperBound() << std::endl;

  plotTheTrajectory(ds);

  return 0;
}


