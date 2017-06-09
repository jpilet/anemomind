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
    LocalModel model;
    int lastUpdate = 0;
    int index = 0;
    int insertedCount = 0;

    LocalState(
        int i,
        int itemCount, const T& x)
      : model(x), lastUpdate(itemCount), index(i) {}
  };

  // Inserts an item and assigns it to an existing or new segment.
  int filter(const T& x) {
    _itemCounter++;

    LocalState newState(_segmentCounter++, _itemCounter, x);
    if (_localModels.size() < _settings.maxLocalModelCount) {
      _localModels.push_back(newState);
    } else {
      _localModels[oldestIndex()] = newState;
    }
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
