#include <device/anemobox/BinarySignal.h>

#include <algorithm>
#include <iterator>

namespace sail {

namespace {

struct EdgeMergeIterator : public std::iterator<std::output_iterator_tag, void,  std::ptrdiff_t, void *, void> {
  EdgeMergeIterator(BinarySignal * container) : container(container), count(0) { }
  EdgeMergeIterator& operator= (const TimedValue<BinaryEdge>& x) {
    int delta = (x.value == BinaryEdge::ToOn ? 1 : -1);
    if ((count == 0 && delta == 1) || (count == 1 && delta == -1)) {
      container->push_back(x);
    }
    count += delta;
    return *this;
  }
  EdgeMergeIterator& operator* () { return *this; }
  EdgeMergeIterator& operator++ () { return *this; }
  EdgeMergeIterator operator++ (int) { return *this; }

  int count;
  BinarySignal* container;
};

}  // namespace

BinarySignal binarySignalUnion(const BinarySignal& a, const BinarySignal& b) {
  if (a.size() == 0) {
    return b;
  }
  if (b.size() == 0) {
    return a;
  }

  BinarySignal merged;

  std::merge(a.begin(), a.end(), b.begin(), b.end(),
             EdgeMergeIterator(&merged));
  return merged;
}


}  // namespace sail
