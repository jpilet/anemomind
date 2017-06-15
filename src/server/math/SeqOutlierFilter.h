/*
 * SeqOutlierFilter.h
 *
 *  Created on: 9 Jun 2017
 *      Author: jonas
 */

#ifndef SERVER_MATH_SEQOUTLIERFILTER_H_
#define SERVER_MATH_SEQOUTLIERFILTER_H_

#include <server/common/indexed.h>
#include <server/common/logging.h>
#include <server/common/Span.h>
#include <set>

namespace sail {
namespace SeqOutlierFilter {

struct Settings {
  int maxLocalModelCount = 0;
  double maxLocalModelCost = 0;
};


// This State template class corresponds to a
// the state of a causal filter. Its most important
// method is 'filter', that updates the filter with
// an observation, and returns an index corresponding to
// a segment that observation is assigned to. The idea is
// that outliers will get assigned different segment indices
// than inliers, and inliers, in turn form long segments.
//
// So, given a sequence of observations, we get a sequence of segment
// indices, one index per observation. In a seconds step, that sequence
// can be parsed with a heuristic that identifies inlier segments.
//
// Template parameters:
// A LocalModel type has the following methods:
//
//   // Adds a new item to the model and updates
//   // the model in place to fit to that item, too.
//   void insert(const T& x);
//
//   // Evalutes the fitness cost of the model
//   // if we *would* insert x. (but doesn't insert it).
//   double previewCost(const T& x) const;
//
// The type T is the type of observation. It could, for instance,
// be a TimedValue<GeographicPosition<double>> in case we were to
// filter GPS data.
template <typename T, typename LocalModel>
class State {
public:
  State(
      const Settings& settings,
      const LocalModel& prototype)
    : _settings(settings), _prototype(prototype) {
      CHECK(0 < settings.maxLocalModelCost);
      CHECK(0 < settings.maxLocalModelCount);
  }

  struct LocalState {
    // The index of its associated segment.
    int index = 0;

    // The current local model.
    LocalModel model;

    // When the last update happened. Used to identify the oldest
    // LocalState that we replace by a new one in case we need
    // a new state.
    int lastUpdate = 0;

    // How many times we have inserted something.
    int insertedCount = 0;

    LocalState(
        const LocalModel& prototype,
        int i,
        int itemCount, const T& x)
      : model(prototype), lastUpdate(itemCount), index(i) {
      model.insert(x);
    }
  };

  struct Candidate {
    LocalState* stateToUpdate = nullptr;
    double selCost = std::numeric_limits<double>::infinity();

    Candidate() {}
    Candidate(double maxCost, LocalState* state, const T& x)
        : stateToUpdate(state) {
      auto nextCost = state->model.previewCost(x);

      // Check if the fitness to the local model
      // is good enough.
      if (nextCost < maxCost) {

        /*
         * Here we decide how to choose the best state to
         * update. For instance, we can choose the best fit. Or
         * we can choose the longest segment.
         *
         * If we choose the longest segment we will greedily
         * try to make long segments longer.
         */
        selCost = -state->insertedCount;
      }
    }

    bool operator<(const Candidate& other) const {
      return selCost < other.selCost;
    }
  };

  Candidate getModelToUpdate(const T& x) {
    Candidate best;
    for (auto& m: _localStates) {
      best = std::min(best,
          Candidate(_settings.maxLocalModelCost,
              &m, x));
    }
    return best;
  }

  // Inserts an item and assigns it to an existing or new segment.
  int filter(const T& x) {
    _itemCounter++;

    // First try to see if there is already an acceptable local state,
    // and in that case, choose the best state by some criterion.
    auto best = getModelToUpdate(x);
    if (best.stateToUpdate) {
      auto& u = *(best.stateToUpdate);

      // Actually insert it
      u.model.insert(x);
      u.lastUpdate = _itemCounter;
      u.insertedCount++;
      return u.index;
    }

    LocalState newState(
        _prototype, _segmentCounter++, _itemCounter, x);
    if (_localStates.size() < _settings.maxLocalModelCount) {
      _localStates.push_back(newState);
    } else {
      _localStates[nextToReplace()] = newState;
    }
    return newState.index;
  }
private:
  Settings _settings;
  int _segmentCounter = 0;
  int _itemCounter = 0;
  LocalModel _prototype;
  std::vector<LocalState> _localStates;

  int nextToReplace() const {
    // Not sure which rule is best to determine what to
    // replace next.
    return oldestIndex();
  }

  int oldestIndex() const {
    auto luAndIndex = std::make_pair(_itemCounter, -1);
    for (const auto& x: indexed(_localStates)) {
      luAndIndex = std::min(
          luAndIndex,
            std::make_pair(x.second.lastUpdate, x.first));
    }
    return luAndIndex.second;
  }
};

struct IndexSpan {
  Span<int> span;
  int index = -1;
};

class IndexGrouper {
public:
  void insert(int index);
  Array<IndexSpan> get();
private:
  int _counter = 0;
  int _lastIndex = -1;
  int _lastFrom = 0;
  ArrayBuilder<IndexSpan> _groups;
  void flush();
};

std::set<int> computeInlierSegments(
    const Array<IndexSpan>& spans);

}
}






#endif /* SERVER_MATH_SEQOUTLIERFILTER_H_ */
