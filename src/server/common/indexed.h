/*
 * indexed.h
 *
 *  Created on: 12 Nov 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_INDEXED_H_
#define SERVER_COMMON_INDEXED_H_

#include <server/common/Functional.h>

namespace sail {

template <typename Container>
class IndexedContainer {
public:
  typedef decltype(copyOf(std::declval<Container>()[0])) value_type;
  typedef IndexedContainer<Container> ThisType;

  IndexedContainer(const Container &c) : _c(c) {}

  struct Iterator {
    int index = 0;
    const Container *c;

    Iterator(int i, const Container *c_) : index(i), c(c_) {}

    Iterator &operator++() {
      index++;
      return *this;
    }

    bool operator==(const Iterator &x) const {
      return (index == x.index) && (c == x.c);
    }

    bool operator!=(const Iterator &x) const {
      return !(*this == x);
    }

    std::pair<int, value_type> operator*() const {
      return std::make_pair(index, (*c)[index]);
    }

    operator bool() const {
      return c != nullptr;
    }
  };

  Iterator begin() const {
    return Iterator(0, &_c);
  }

  Iterator end() const {
    return Iterator(_c.size(), &_c);
  }
private:
  const Container &_c;
};

template <typename Container>
IndexedContainer<Container> indexed(const Container &c) {
  return IndexedContainer<Container>(c);
}

}



#endif /* SERVER_COMMON_INDEXED_H_ */
