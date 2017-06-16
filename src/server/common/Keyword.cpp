/*
 * Keyword.cpp
 *
 *  Created on: 3 Aug 2017
 *      Author: jonas
 */

#include <server/common/Keyword.h>
#include <server/common/logging.h>
#include <map>

namespace sail {

namespace {

  std::string undefinedName = "";

  class Allocator {
  public:
    size_t allocate(const std::string& name);

    std::string getName(size_t i) const;
  private:
    mutable std::mutex _mutex;
    size_t _counter = 0;
    std::map<size_t, std::string> _index2name;
    std::map<std::string, size_t> _name2index;
  };

  size_t Allocator::allocate(const std::string& name) {
    if (name == undefinedName) {
      return Keyword::UndefinedIndex;
    }
    std::lock_guard<std::mutex> lock(_mutex);
    auto f = _name2index.find(name);
    if (f == _name2index.end()) {
      auto i = _counter;
      _index2name[i] = name;
      _name2index[name] = i;
      _counter++;
      return i;
    } else {
      return f->second;
    }
  }

  std::string Allocator::getName(size_t i) const {
    if (i == Keyword::UndefinedIndex) {
      return undefinedName;
    }
    std::lock_guard<std::mutex> lock(_mutex);
    auto f = _index2name.find(i);
    CHECK(f != _index2name.end());
    return f->second;
  }

  Allocator* getAllocator() {
    static Allocator a;
    return &a;
  }
}


Keyword Keyword::make(const std::string& name) {
  return Keyword(getAllocator()->allocate(name));
}

std::string Keyword::name() const {
  return getAllocator()->getName(_index);
}


} /* namespace sail */
