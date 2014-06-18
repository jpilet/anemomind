/*
 *  Created on: 2014-06-18
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/hmm/StateAssignOps.h>
#include <gtest/gtest.h>
#include <server/common/Uniform.h>
#include <server/common/ArrayIO.h>
#include <server/common/SharedPtrUtils.h>

using namespace sail;

namespace {
  const int stateCount = 6;

  bool isBeforeRace(int stateIndex) {
    return stateIndex == 0 || stateIndex == 1;
  }

  bool isRacing(int stateIndex) {
    return stateIndex == 2 || stateIndex == 3;
  }

  bool isAfterRace(int stateIndex) {
    return stateIndex == 4 || stateIndex == 5;
  }

  const int len = 30;
  int preferredStates[len] = {5, 2, 4, 4, 5, 1, 2, 4, 1, 3, 2, 3, 2, 3, 5, 5, 3, 4, 0, 3, 0, 1, 0, 4, 0, 2, 0, 0, 5, 1};

  const int pb = 2;
  int predsBefore[pb] = {0, 1};

  const int pi = 4;
  int predsIn[pi] = {0, 1, 2, 3};

  const int pa = 4;
  int predsAfter[pa] = {2, 3, 4, 5};

  class SomeRace : public StateAssign {
   public:
    virtual double getStateCost(int stateIndex, int timeIndex) {
      return (stateIndex == preferredStates[timeIndex]? 0 : 1);
    }

    virtual double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {
      return 0;
    }

    int getStateCount() {
      return stateCount;
    }

    int getLength() {
      return len;
    }

    Arrayi getPrecedingStates(int stateIndex, int timeIndex) {
      if (isBeforeRace(stateIndex)) {
        return Arrayi(pb, predsBefore);
      } else if (isRacing(stateIndex)) {
        return Arrayi(pi, predsIn);
      } else {
        return Arrayi(pa, predsAfter);
      }
    }

   private:
  };

  int getRaceStartIndex(Arrayi states) {
    int count = states.size();
    for (int i = 0; i < count; i++) {
      if (isRacing(states[i])) {
        return i;
      }
    }
    return -1;
  }

  int getRaceEndIndex(Arrayi states) {
    int count = states.size();
    for (int i = 0; i < count; i++) {
      if (isAfterRace(states[i])) {
        return i;
      }
    }
    return -1;
  }

  double huge = 1.0e7;

  class RaceHint : public StateAssignHint {
   public:
    RaceHint(int at, bool starts) : _at(at), _starts(starts) {}
    double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {
      if (fromTimeIndex+1 == _at) {
        if (_starts) {
          return (isBeforeRace(fromStateIndex) && isRacing(toStateIndex)? 0 : huge);
        } else {
          return (isRacing(fromStateIndex) && isAfterRace(toStateIndex)? 0 : huge);
        }
      }
      return 0;
    }

    double getStateCost(int stateIndex, int timeIndex) {
      return 0;
    }

    int getLength() {return len;}
    int getStateCount() {return stateCount;}
   private:
    int _at;
    bool _starts;
  };
}

TEST(StateAssignOpsTest, SimpleParsingWithHints) {
  const int raceStartsAt = 10;
  const int raceEndsAt = 20;

  SomeRace parser;
  Arrayi withoutHints = parser.solve();
  EXPECT_NE(getRaceStartIndex(withoutHints), raceStartsAt);
  EXPECT_NE(getRaceEndIndex(withoutHints), raceEndsAt);

  std::shared_ptr<StateAssign> hintedParser = makeSharedPtrToStack(parser)
      + std::shared_ptr<StateAssign>(new RaceHint(raceStartsAt, true))
      + std::shared_ptr<StateAssign>(new RaceHint(raceEndsAt, false));

  Arrayi withHints = hintedParser->solve();
  EXPECT_EQ(getRaceStartIndex(withHints), raceStartsAt);
  EXPECT_EQ(getRaceEndIndex(withHints), raceEndsAt);
}


