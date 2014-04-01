/*
 *  Created on: 13 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef GRIDFITTERTESTDATA_H_
#define GRIDFITTERTESTDATA_H_

#include <server/common/Array.h>

namespace sail {

class GridFitterTestData {
 public:
  GridFitterTestData();

  int sampleCount;
  Arrayd X, Ygt, Ynoisy;

  Array<Arrayb> splits;
};

} /* namespace sail */

#endif /* GRIDFITTERTESTDATA_H_ */
