#include <server/nautical/ValidPeriods.h>

#include <algorithm>

namespace sail {

namespace {

struct StatusMergeIterator {
  StatusMergeIterator(StatusTimedVector * container) : container(container), count(0) { }
  StatusMergeIterator& operator= (const TimedValue<StatusChange>& x) {
    int delta = (x.value == StatusChange::toValid ? 1 : -1);
    if ((count == 0 && delta == 1) || (count == 1 && delta == -1)) {
      container->push_back(x);
    }
    count += delta;
    return *this;
  }
  StatusMergeIterator& operator* () { return *this; }
  StatusMergeIterator& operator++ () { return *this; }
  StatusMergeIterator operator++ (int) { return *this; }

  int count;
  StatusTimedVector* container;
};

}  // namespace

StatusTimedVector statusVectorUnion(
    const StatusTimedVector& a, const StatusTimedVector& b) {
  if (a.size() == 0) {
    return b;
  }
  if (b.size() == 0) {
    return a;
  }

  StatusTimedVector merged;

  std::merge(a.begin(), a.end(), b.begin(), b.end(),
             StatusMergeIterator(&merged));
  return merged;
}


}  // namespace sail
