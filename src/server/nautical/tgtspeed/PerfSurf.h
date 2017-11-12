/*
 * PerfSurf.h
 *
 *  Created on: 12 Nov 2017
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_PERFSURF_H_
#define SERVER_NAUTICAL_PERFSURF_H_

namespace sail {

struct WeightedIndex {
  int index = 0;
  double weight = 1.0;
};

struct PerfSurfPt {

};

class PerfSurf {
public:
  PerfSurf();
  virtual ~PerfSurf();
};

} /* namespace sail */

#endif /* SERVER_NAUTICAL_PERFSURF_H_ */
