/*
 * SeqOutlierFilter.h
 *
 *  Created on: 9 Jun 2017
 *      Author: jonas
 */

#ifndef SERVER_MATH_SEQOUTLIERFILTER_H_
#define SERVER_MATH_SEQOUTLIERFILTER_H_

#include <server/common/indexed.h>

namespace sail {
namespace SeqOutlierFilter {

struct Settings {
  int maxLocalModelCount = 0;
  double maxLocalModelCost = 0;
};


// A local model has the following methods:
//
//   // Adds a new item to the model
//   LocalModel insert(const T& x) const;
//
//   // Evalutes the cost of fitting the local model
//   double cost() const;
template <typename T, typename LocalModel>
class State {
public:
  State(
      const Settings& settings,
      const LocalModel& prototype)
    : _settings(settings), _prototype(prototype) {}

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
        int i,
        int itemCount, const T& x)
      : model(x), lastUpdate(itemCount), index(i) {}
  };

  struct Candidate {
    LocalModel* stateToUpdate = nullptr;
    T nextModel;

    Candidate() {}
    Candidate(double maxCost, LocalState* state, const T& x) {
      auto next = state->model.insert(x);
      auto nextCost = next.cost();

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

    double selCost = std::numeric_limits<double>::infinity();

    bool operator<(const Candidate& other) {
      return selCost < other.selCost;
    }
  };

  Candidate getModelToUpdate(const T& x) {
    Candidate best;
    for (auto& m: _localModels) {
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
    if (x.stateToUpdate) {
      auto& u = *(x.stateToUpdate);
      u.model = x.nextModel;
      u.lastUpdate = _itemCounter;
      u.insertedCount++;
      return u.index;
    }

    LocalState newState(_segmentCounter++, _itemCounter, x);
    if (_localModels.size() < _settings.maxLocalModelCount) {
      _localModels.push_back(newState);
    } else {
      _localModels[oldestIndex()] = newState;
    }
    return newState.index;
  }
private:
  Settings _settings;
  int _segmentCounter = 0;
  int _itemCounter = 0;
  LocalModel _prototype;
  std::vector<LocalModel> _localModels;

  int oldestIndex() const {
    auto luAndIndex = std::make_pair(_itemCounter, -1);
    for (const auto& x: indexed(_localModels)) {
      luAndIndex = std::min(
          luAndIndex,
            std::make_pair(x.second.lastUpdate, x.first));
    }
    return luAndIndex.second;
  }
};


}
}






#endif /* SERVER_MATH_SEQOUTLIERFILTER_H_ */
