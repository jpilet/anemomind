/*
 * WeightedIndex.h
 *
 *  Created on: 16 Mar 2018
 *      Author: jonas
 */

#ifndef SERVER_COMMON_WEIGHTEDINDEX_H_
#define SERVER_COMMON_WEIGHTEDINDEX_H_

namespace sail {
  /*
   * A weighted index is a struct used to encoding a sparse linear combination
   * of vertices in order to parameterize a point. Barycentric coordinates in
   * computer graphics are a special case of that.
   */
  struct WeightedIndex {
    int index = 0;
    double weight = 1.0;

    WeightedIndex() {}
    WeightedIndex(int i, double w) : index(i), weight(w) {}
  };
}




#endif /* SERVER_COMMON_WEIGHTEDINDEX_H_ */
