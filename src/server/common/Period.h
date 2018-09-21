#ifndef COMMON_PERIOD_H
#define COMMON_PERIOD_H

#include <server/common/TimeStamp.h>
#include <algorithm>

namespace sail {

struct Period {
  TimeStamp begin;
  TimeStamp end;

  Period() { }
  Period(TimeStamp b, TimeStamp e) : begin(b), end(e) { }
  Period(TimeStamp b, Duration<> d) : begin(b), end(b + d) { }

  Duration<> duration() const { return end - begin; }

  bool defined() const { return begin.defined(); }

  void restrict(const Period& other) {
    begin = std::max(begin, other.begin);
    end = std::max(begin, std::min(end, other.end));
  }
};

}  // namespace sail

#endif  // COMMON_PERIOD_H
