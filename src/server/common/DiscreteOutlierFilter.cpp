/*
 * DiscreteOutlierFilter.cpp
 *
 *  Created on: 4 Nov 2016
 *      Author: jonas
 */

#include <server/common/DiscreteOutlierFilter.h>

namespace sail {
namespace DiscreteOutlierFilter {

Array<double> computeOutlierMaskFromPairwiseCosts(
    const Array<double> &pairwiseCosts, const Settings &settings) {
  int pairCount = pairwiseCosts.size();
  int n = pairCount + 1;
  ArrayBuilder<int> splits(pairCount);
  splits.add(0);
  for (int i = 0; i < pairCount; i++) {
    if (settings.threshold < pairwiseCosts[i]) {
      splits.add(i + 1);
    }
  }
  splits.add(n);
}

}
}
