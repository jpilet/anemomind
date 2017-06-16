/*
 * Keyword.h
 *
 *  Created on: 3 Aug 2017
 *      Author: jonas
 */

#ifndef SERVER_COMMON_KEYWORD_H_
#define SERVER_COMMON_KEYWORD_H_

#include <stdint.h>
#include <limits>
#include <string>

namespace sail {

// Similar to a string, but optimized
// to be used as a key referring to things,
// rather than storing character data.
//
// Construction and retrieving the underlying string
// is relatively slow, but once a Keyword has been constructed,
// comparisons and copies are very cheap. The idea is to use
// the Keyword in situations where we mostly need fast comparisons,
// small space usage and will not need to use the more
// expensive operations very often.
//
// Unlike a string, the underlying space required is
// only that of size_t. When running a program,
// make sure to limit the number of Keywords constructed,
// because the data associated with a Keyword will live forever
// and if we don't limit the construction of Keywords, we will
// run out of memory.
class Keyword {
public:
  static const size_t UndefinedIndex =
      std::numeric_limits<size_t>::max();

  Keyword() {}

  static Keyword make(const std::string& name);
  std::string name() const;

  bool operator==(const Keyword& other) const {
    return _index == other._index;
  }

  bool operator!=(const Keyword& other) const {
    return _index != other._index;
  }

  size_t runtimeAssignedIndex() const {
    return _index;
  }

  bool undefined() const {
    return _index == UndefinedIndex;
  }

  bool defined() const {
    return !undefined();
  }
private:
  Keyword(size_t s) : _index(s) {}

  size_t _index = UndefinedIndex;
};

#define DECLARE_KEYWORD(x) auto x = sail::Keyword::make(#x)


} /* namespace sail */

namespace std {
  template <> struct hash<sail::Keyword>
  {
    size_t operator()(const sail::Keyword& x) const {
      return x.runtimeAssignedIndex();
    }
  };
}

#endif /* SERVER_COMMON_KEYWORD_H_ */
