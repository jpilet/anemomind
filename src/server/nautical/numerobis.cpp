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
      std::cout << "\n   Gap of " << (next.minv() - span.maxv()).str() << "\n\n";
    }
  }
}

void plotTheTrajectory(const NavDataset &ds) {
  std::cout << "GPS speed time stamps" << std::endl;
  showTimeSpans(listSpans(ds.samples<GPS_SPEED>()));
  std::cout << "GPS pos time stamps" << std::endl;
  auto spans = listSpans(ds.samples<GPS_POS>());
  showTimeSpans(spans);
  std::cout << "Showed it" << std::endl;

  auto allgps = ds.samples<GPS_POS>();
  auto mid = allgps[allgps.size()/2];
  GeographicReference geoRef(mid.value);

  GnuplotExtra plot;

  for (auto span: spans) {
    if (TimeStamp::UTC(2016, 5, 19, 10, 43, 0) < span.minv()) {
      auto subset = ds.slice(span.minv(), span.maxv());
      auto gps = subset.samples<GPS_POS>();
      int n = gps.size();
      MDArray2d pts(n, 2);
      int counter = 0;
      for (int i = 0; i < n; i++) {
        auto xy = geoRef.map(gps[i].value);
        auto x = xy[0].meters();
        auto y = xy[1].meters();
        if (sqrt(sqr(x) + sqr(y)) < 100000) {
          pts(counter, 0) = x;
          pts(counter, 1) = y;
          counter++;
        }
      }
      pts = pts.sliceRowsTo(counter);

      std::stringstream ss;
      ss << "From " << span.minv() << " to " << span.maxv();

      std::cout << "Plot " << ss.str() << std::endl;

      plot.set_style("lines");
      plot.plot(pts, ss.str());
    }
  }

  {
    auto lastPos = allgps.last();
    plot.set_style("lines");
    auto xy = geoRef.map(lastPos.value);
    auto k = 100;
    MDArray2d A(2, 2), B(2, 2);
    A(0, 0) = xy[0].meters() - k;
    A(0, 1) = xy[1].meters() - k;
    A(1, 0) = xy[0].meters() + k;
    A(1, 1) = xy[1].meters() + k;

    B(0, 0) = xy[0].meters() - k;
    B(0, 1) = xy[1].meters() + k;
    B(1, 0) = xy[0].meters() + k;
    B(1, 1) = xy[1].meters() - k;

    plot.plot(A);
    plot.plot(B);
  }



  plot.show();
}



int main() {
  std::string dir = "/home/jonas/prog/anemomind/datasets/boat573b14bbf437a48c0ce3ad54";

  LogLoader loader;
  loader.load(dir);

  auto ds = loader.makeNavDataset().fitBounds();

  ds.outputSummary(&(std::cout));

  std::cout << "Loaded logs from " << ds.lowerBound() << " to " << ds.upperBound() << std::endl;

  plotTheTrajectory(ds);

  return 0;
}


