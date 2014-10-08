/*
 *  Created on: 2014-09-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/hmm/QuantFilter.h>
#include <server/common/math.h>
#include <server/math/hmm/StateAssign.h>
#include <server/common/logging.h>
#include <server/common/string.h>
#include <iostream>

namespace sail {


namespace {
  class Stepper : public StateAssign {
   public:
    Stepper(Array<LineKM> binMap,
            MDArray2d noisyData,
            double regularization);

    double getStateCost(int stateIndex, int timeIndex);
    double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex);

    int getStateCount() {
      return _stateCount;
    }

    int getLength() {
      return _length;
    }

    Arrayi getPrecedingStates(int stateIndex, int timeIndex) {
      return _allInds;
    }

    int update(Arrayi inds);

    int step() {
      return update(solve());
    }

    const MDArray2i &state() const {
      return _state;
    }
   private:
    Arrayi _allInds;

    int _stateCount, _dims, _length;
    MDArray2d _data;
    Array<LineKM> _map;
    double _reg;

    MDArray2i _state;

    const Arrayi &getTempInds(int stateIndex, int timeIndex);

    // Returns true if the state was updated
    bool updateState(int stateIndex, int timeIndex, int newValue);
  };

  Stepper::Stepper(Array<LineKM> binMap,
          MDArray2d noisyData,
          double regularization) :
    _map(binMap), _data(noisyData), _reg(regularization),
    _length(noisyData.cols()),
    _stateCount(2*binMap.size() + 1),
    _dims(binMap.size()) {
    _allInds = listStateInds();
    int dims = binMap.size();
    assert(dims == noisyData.rows());
    assert(regularization >= 0);

    _state = MDArray2i(_dims, _length);
    _state.setAll(0);
  }

  double Stepper::getStateCost(int stateIndex, int timeIndex) {
    const Arrayi &inds = getTempInds(stateIndex, timeIndex);
    double cost = 0;
    for (int i = 0; i < _dims; i++) {
      cost += sqr(_data(i, timeIndex) - _map[i](inds[i]));
    }
    return cost;
  }

  double Stepper::getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {
    const Arrayi &from = getTempInds(fromStateIndex, fromTimeIndex);
    const Arrayi &to = getTempInds(toStateIndex, fromTimeIndex + 1);
    double cost = 0;
    for (int i = 0; i < _dims; i++) {
      cost += sqr(_map[i].getK()*(from[i] - to[i]));
    }
    return _reg*sqrt(cost);
  }

  int Stepper::update(Arrayi inds) {
    assert(inds.size() == _length);
    int counter = 0;
    for (int j = 0; j < _length; j++) {
      const Arrayi &newInds = getTempInds(inds[j], j);
      for (int i = 0; i < _dims; i++) {
        counter += (updateState(i, j, newInds[i])? 1 : 0);
      }
    }
    return counter;
  }

  const Arrayi &Stepper::getTempInds(int stateIndex, int timeIndex) {
    static Arrayi tempinds2[2] = {Arrayi(_stateCount), Arrayi(_stateCount)};
    Arrayi &tempinds = tempinds2[timeIndex % 2];

    for (int i = 0; i < _dims; i++) {
      tempinds[i] = _state(i, timeIndex);
    }

    const int noChange = _stateCount-1;
    if (stateIndex != noChange) {
      int index = stateIndex/2;
      int change = 2*(stateIndex - 2*index) - 1;
      assert(change == -1 || change == 1);
      tempinds[index] += change;
    }
    return tempinds;
  }

  bool Stepper::updateState(int stateIndex, int timeIndex, int newValue) {
    int &dst = _state(stateIndex, timeIndex);
    if (dst != newValue) {
      dst = newValue;
      return true;
    }
    return false;
  }
}

MDArray2i quantFilter(Array<LineKM> binMap,
                        MDArray2d noisyData,
                        double regularization, int maxIter) {
  Stepper stepper(binMap, noisyData, regularization);
  LOG(INFO) << "Running quantFilter...";
  int counter = 0;
  for (int i = 0; i < maxIter; i++) {
    counter++;
    int updates = stepper.step();
    LOG(INFO) << "   quantFilter iteration " << i << ". Updates: " << updates;
    if (updates == 0) {
      break;
    }
  }
  LOG(INFO) << "quantFilter required " << counter << " iterations.";
  return stepper.state();
}

MDArray2i quantFilterChunked(Array<LineKM> binMap,
    MDArray2d noisyData,
    double regularization,
    int chunkSize, int maxIter) {
    if (chunkSize <= 0) {
      return quantFilter(binMap, noisyData, regularization, maxIter);
    } else {
      int count = noisyData.cols();
      int chunkCount = (count - 1)/chunkSize + 1;
      LOG(INFO) << "Runing quantFilterChunked...";
      MDArray2i dst(noisyData.rows(), count);
      for (int i = 0; i < chunkCount; i++) {
        LOG(INFO) << " Chunk " << i << " of " << chunkCount;
        int from = i*chunkSize;
        int to = std::min(count, from + chunkSize);
        quantFilter(binMap, noisyData.sliceCols(from, to), regularization, maxIter)
          .copyToSafe(dst.sliceCols(from, to));
      }
      LOG(INFO) << "Done quantFilterChunked.";
      return dst;
    }
}



}
