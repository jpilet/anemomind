#ifndef COMMON_MULTIMERGE
#define COMMON_MULTIMERGE

/*
 * Merge multiple sorted collections into a single sorted stream.
 * Complexity: O(N * log n) where N is the total number of elements
 * and n is the number of merged streams.
 *
 * See MultiMergeTest.cpp for a usage example.
 */
#include <assert.h>
#include <functional>
#include <queue>  // std::priority_queue
#include <vector>

#include <server/common/Optional.h>

namespace sail {

template <typename T>
class SortedStream {
 public:
  virtual ~SortedStream<T>() { }
  virtual T value() const = 0;
  virtual bool next() = 0;
  virtual bool end() const = 0;
};

template <typename T, class Compare = std::less<T>>
class MultiMerge : public SortedStream<T> {
 public:
  MultiMerge() { }

  void addStream(SortedStream<T>* stream) {
    assert(stream);
    if (!stream->end()) {
      _queue.push(stream);
    }
  }

  virtual T value() const { return _queue.top()->value(); }
  virtual bool next() {
    SortedStream<T> *popped = _queue.top();
    _queue.pop();
    popped->next();
    addStream(popped);
    return end();
  }

  virtual bool end() const { return _queue.empty(); }

 private:
 
  struct CompareStreamPtr {
    bool operator() (const SortedStream<T>* x,
                     const SortedStream<T>* y) const {
      Compare cmp;
      // The priority queue returns the highest value first.
      // But we want the lowest one. So we have to swap arguments
      // because we want the template argument Compare to behave
      // as with "sort" (with "<" you get lowest values first)
      return cmp(y->value(), x->value());
    }
  };

  std::priority_queue<
    SortedStream<T> *,
    std::vector<SortedStream<T> *>, CompareStreamPtr> _queue;
};

}  // namespace sail

#endif  // COMMON_MULTIMERGE
