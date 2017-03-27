#ifndef NAUTICAL_VALID_PERIOD_H
#define NAUTICAL_VALID_PERIOD_H

#include <device/anemobox/TimedSampleCollection.h>
#include <server/common/logging.h>

namespace sail {

enum StatusChange {
  toInvalid,
  toValid
};

using StatusTimedVector = TimedSampleCollection<StatusChange>::TimedVector;

// Combine two StatusTimedVector in a single one so that the result is the union
// of valid periods of a and b.
StatusTimedVector statusVectorUnion(
    const StatusTimedVector& a, const StatusTimedVector& b);

struct Period {
  TimeStamp begin;
  TimeStamp end;

  Period(TimeStamp b, TimeStamp e) : begin(b), end(e) { }
  Duration<> duration() const { return end - begin; }
};

// a class temporarily wrapping a StatusTimedVector to make it look as a
// collection of Period.
// Typical usage:
//
//   for (Period p : ValidPeriods(&statusTimedVector)) {
//      cout << "Starts at " << p.begin << " and last " << p.duration() << endl;
//   }
//
class ValidPeriods {
 public:
  explicit ValidPeriods(const StatusTimedVector* status) : _status(status) {
    if (status->size() > 0) {
      CHECK_EQ(StatusChange::toValid, status->front().value);
      CHECK_EQ(StatusChange::toInvalid, status->back().value);
    }
  }

  struct iterator {
    iterator() { }
    iterator(StatusTimedVector::const_iterator i) : it(i) { }
    iterator(const iterator& a) = default;
    iterator& operator=(const iterator& a) = default;

    Period operator*() const { return getPeriod(); }

    iterator& operator++() { it += 2; return *this; }
    iterator& operator+=(int delta) { it += 2 * delta; return *this; }

    Period getPeriod() const {
      CHECK_EQ(StatusChange::toValid, it->value);
      auto end = it + 1;
      CHECK_EQ(StatusChange::toInvalid, end->value);

      return Period(it->time, end->time);
    }
    bool operator == (const iterator& other) { return it == other.it; }
    bool operator != (const iterator& other) { return it != other.it; }

    StatusTimedVector::const_iterator it;
  };

  iterator begin() const {
    return iterator(_status->begin());
  }
  iterator end() const {
    return iterator(_status->end());
  }

  private:
  const StatusTimedVector* _status;
};


}  // namespace

#endif  // NAUTICAL_VALID_PERIOD_H
