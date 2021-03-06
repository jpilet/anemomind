#ifndef ANEMOBOX_BINARY_SIGNAL_H_
#define ANEMOBOX_BINARY_SIGNAL_H_

#include <device/anemobox/TimedSampleCollection.h>
#include <server/common/Period.h>

#include <cassert>
#include <iterator>

namespace sail {

enum class BinaryEdge : unsigned char {
  ToOff = 0,
  ToOn = 1
};

using BinarySignal = TimedSampleCollection<BinaryEdge>::TimedVector;

BinarySignal binarySignalUnion(const BinarySignal& a,
                               const BinarySignal& b);

// a class temporarily wrapping a BinarySignal to make it look as a
// collection of Period.
// Typical usage:
//
//   for (Period p : OnPeriods(&signal)) {
//      cout << "Starts at " << p.begin << " and last " << p.duration() << endl;
//   }
//
class OnPeriods {
 public:
  explicit OnPeriods(const BinarySignal* signal) : _signal(signal) {
    if (signal->size() > 0) {
      assert(BinaryEdge::ToOn == signal->front().value);
      assert(BinaryEdge::ToOff == signal->back().value);
    }
  }

  struct iterator : public std::iterator<std::forward_iterator_tag, Period> {
    iterator() { }
    iterator(BinarySignal::const_iterator i) : it(i) { }
    iterator(const iterator& a) = default;
    iterator& operator=(const iterator& a) = default;

    Period operator*() const { return getPeriod(); }

    iterator& operator++() { it += 2; return *this; }
    iterator& operator+=(int delta) { it += 2 * delta; return *this; }

    Period getPeriod() const {
      assert(BinaryEdge::ToOn == it->value);
      auto end = it + 1;
      assert(BinaryEdge::ToOff == end->value);

      return Period(it->time, end->time);
    }
    bool operator == (const iterator& other) const { return it == other.it; }
    bool operator != (const iterator& other) const { return it != other.it; }

    BinarySignal::const_iterator it;
  };

  iterator begin() const {
    return iterator(_signal->begin());
  }
  iterator end() const {
    return iterator(_signal->end());
  }

  private:
  const BinarySignal* _signal;
};

}  // namespace sail

#endif  // ANEMOBOX_BINARY_SIGNAL_H_
