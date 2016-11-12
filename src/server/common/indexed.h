/*
 * indexed.h
 *
 *  Created on: 12 Nov 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_INDEXED_H_
#define SERVER_COMMON_INDEXED_H_

namespace sail {

template <typename Container>
auto indexed(const Container &c) {
  typedef decltype(c[0]) T;
  return sail::map(Span<int>(0, c.size()), c,
      [](int index, T x) {
    return std::make_pair(index, x);
  });
}

}



#endif /* SERVER_COMMON_INDEXED_H_ */
