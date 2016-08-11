/*
 * numerobisTest.cpp
 *
 *  Created on: 11 Aug 2016
 *      Author: jonas
 */

#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/filters/SmoothGpsFilter.h>
#include <server/plot/extra.h>

using namespace sail;

void outputLocalPositions(
		std::string filename,
		TimeStamp refTime,
	const Array<CeresTrajectoryFilter::Types<2>
		::TimedPosition> &X) {
	std::ofstream file(filename);
	for (auto x: X) {
		file << (x.time - refTime).seconds()
				<< " " << x.value[0].meters()
				<< " " << x.value[1].meters() << std::endl;
	}
}

MDArray2d getPositions(Array<CeresTrajectoryFilter::Types<2>
	::TimedPosition> X) {
  MDArray2d dst(X.size(), 2);
  for (int i = 0; i < X.size(); i++) {
    auto x = X[i];
    dst(i, 0) = x.value[0].meters();
    dst(i, 1) = x.value[1].meters();
  }
  return dst;
}

int main() {
	LogLoader loader;
	loader.load("/Users/jonas/data/boat573b14bbf437a48c0ce3ad54");
	auto ds0 = loader.makeNavDataset();
	auto ds = ds0.slice(
		TimeStamp::UTC(2016, 5, 19, 13, 52, 8),
		TimeStamp::UTC(2016, 5, 21, 20, 38, 51));

	std::cout << "Sliced it up and picked the relevant portion." << std::endl;
	std::cout << "Bounded from "
			<< ds.lowerBound() << " to "
			<< ds.upperBound() << std::endl;

	auto results = filterGpsData(ds);

	auto refTime = TimeStamp::UTC(2016, 5, 20, 13, 50, 8);

	outputLocalPositions("/tmp/numerobis_raw.txt",
			refTime, results.rawLocalPositions);
	outputLocalPositions("/tmp/numerobis_filtered.txt",
			refTime, results.filteredLocalPositions);

	GnuplotExtra plot;
	plot.plot(getPositions(results.rawLocalPositions));
	plot.show();

	return 0;
}


