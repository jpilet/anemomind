/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef SERVER_NAUTICAL_NAVDATASET_H_
#define SERVER_NAUTICAL_NAVDATASET_H_

namespace sail {

class NavDataset {
public:
  NavDataset(const std::shared_ptr<Dispatcher> dispatcher);

private:
  // Undefined _lowerBound means negative infinity,
  // Undefined _upperBound means positive infinity.
  sail::TimeStamp _lowerBound, _upperBound;
  std::shared_ptr<Dispatcher> _dispatcher;
};

} /* namespace sail */

#endif /* SERVER_NAUTICAL_NAVDATASET_H_ */
