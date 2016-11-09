/*
 * StateAssignTest.cpp
 *
 *  Created on: 24 janv. 2014
 *      Author: jonas
 */

#include "StateAssign.h"
#include <gtest/gtest.h>

using namespace sail;

namespace {
class NoTransitionCost : public StateAssign {
 public:
  NoTransitionCost();
  double getStateCost(int stateIndex, int timeIndex);
  double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {
    return 0;
  }
  int getStateCount() {
    return 2;
  }
  int getLength() {
    return 12;
  }
  Arrayi getPrecedingStates(int stateIndex, int timeIndex) {
    return _pred;
  }
 private:
  Arrayi _pred;
};

double NoTransitionCost::getStateCost(int stateIndex, int timeIndex) {
  if (stateIndex == 0) {
    return 0.1;
  } else {
    int local = (timeIndex % 4) - 2;
    return local;
  }
}

NoTransitionCost::NoTransitionCost() : _pred(listStateInds()) {
}


}

TEST(StateAssignTest, NoTransitionCost) {
  NoTransitionCost test;
  Arrayi result = test.solve();
  EXPECT_TRUE(result.size() == test.getLength());
  for (int i = 0; i < test.getLength(); i++) {
    EXPECT_EQ(
      test.getStateCost(0, i) < test.getStateCost(1, i),
      result[i] == 0) << "index: " << i << "Cost state 0: " << test.getStateCost(0, i) << " cost state 1: " << test.getStateCost(1, i);
  }
}


namespace {
char toChar(int num) {
  const char chars[] = "0123456789";
  return chars[num];
}
}

namespace {
class NoisyStep : public StateAssign {
 public:
  NoisyStep(std::string noisy);
  double getStateCost(int stateIndex, int timeIndex);
  double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex);
  int getStateCount();
  int getLength();
  Arrayi getPrecedingStates(int stateIndex, int timeIndex);
  void useGrammar();
 private:
  double _transitionCost;
  std::string _noisy;
  Array<Arrayi> _preds;
};

NoisyStep::NoisyStep(std::string noisy) : _noisy(noisy), _transitionCost(4.0) {
  _preds.create(2);
  _preds[0] = listStateInds();
  _preds[1] = listStateInds();
}

double NoisyStep::getStateCost(int stateIndex, int timeIndex) {
  return (toChar(stateIndex) == _noisy[timeIndex]? 0.0 : 1.0);
}

double NoisyStep::getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {
  return (fromStateIndex == toStateIndex? 0.0 : _transitionCost);
}

int NoisyStep::getStateCount() {
  return 2;
}

int NoisyStep::getLength() {
  return _noisy.length();
}

Arrayi NoisyStep::getPrecedingStates(int stateIndex, int timeIndex) {
  return _preds[stateIndex];
}


void NoisyStep::useGrammar() {
  _transitionCost = 0.0;

  // Set _preds[0] to an array with a single element 0. This means
  // that the only possible state transition to state 0 is  0 --> 0.
  // In other words, the transition 1 --> 0 is impossible. This
  // lets us model a grammar. An alternative way to achieve this would
  // be to let getTransitionCost(1, 0) return a prohibitively large cost, e.g. 1.0e12,
  // but that could mean a considerable inefficiency if there are many states.
  _preds[0] = Arrayi{0};
}
}

TEST(StateAssignTest, NoisyStep) {
  std::string noisy = "000010010100011111110111001111";
  std::string gt    = "000000000000011111111111111111";

  NoisyStep test(noisy);
  Arrayi result = test.solve();
  EXPECT_TRUE(result.size() == gt.length());
  for (int i = 0; i < result.size(); i++) {
    EXPECT_TRUE(toChar(result[i]) == gt[i]);
  }
}

TEST(StateAssignTest, NoisyStepGrammar) {
  std::string noisy = "000010010100011111110111001111";
  std::string gt    = "000000000000011111111111111111";

  NoisyStep test(noisy);
  test.useGrammar();

  Arrayi result = test.solve();


  EXPECT_TRUE(result.size() == gt.length());
  for (int i = 0; i < result.size(); i++) {
    EXPECT_TRUE(toChar(result[i]) == gt[i]);
  }
}


