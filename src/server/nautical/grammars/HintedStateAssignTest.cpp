/*
 *  Created on: 2014-07-01
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/grammars/HintedStateAssign.h>

#include <algorithm>
#include <cmath>
#include <gtest/gtest.h>
#include <server/common/SharedPtrUtils.h>

using namespace sail;

namespace {
  class Ref : public StateAssign {
   public:
    Ref() : _preds(listStateInds()) {}

    double getStateCost(int stateIndex, int timeIndex) {
      if (stateIndex == 0) {
        return 1.0 + sin(234.989*timeIndex);
      } else {
        return 1.0 + sin(324.34449*timeIndex + 2988.39964);
      }
    }

    double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {
      return std::abs(fromStateIndex - toStateIndex)*100;
    }

    int getStateCount() {return 2;}

    int getLength() {return 30;}

    Arrayi getPrecedingStates(int stateIndex, int timeIndex) {
      return _preds;
    }
   private:
    Arrayi _preds;
  };

  class Hint : public LocalStateAssign {
   public:
    int getStateCount() {return 2;}

    int begin() const { // Corresponds to 0 if it were covering the whole span
      return 15;
    }

    int end() const { // Corresponds to getLength() if it were covering the whole span
      return 17;
    }

    double getStateCost(int stateIndex, int timeIndex) {
      return 0;
    }

    double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {
      if (fromTimeIndex == 15) {
        if (fromStateIndex == 0 && toStateIndex == 1) {
          return 0;
        }
        return 1.0e9;
      }
      return 0.0;
    }
  };

  class Hint2 : public LocalStateAssign {
     public:
      int getStateCount() {return 2;}

      int begin() const { // Corresponds to 0 if it were covering the whole span
        return 19;
      }

      int end() const { // Corresponds to getLength() if it were covering the whole span
        return 21;
      }

      double getStateCost(int stateIndex, int timeIndex) {
        return 0;
      }

      double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {
        if (fromTimeIndex == 19) {
          if (fromStateIndex == 1 && toStateIndex == 0) {
            return 0;
          }
          return 1.0e9;
        }
        return 0.0;
      }
    };
}

TEST(HintedStateAssignTest, HintTest) {
  Ref ref;
  Hint hint;
  Hint2 hint2;

  EXPECT_TRUE(ref.solve().same<int>([=](int x) {return x;}));

  {
    HintedStateAssign hinted(makeSharedPtrToStack(ref),
        Array<HintedStateAssign::LocalStateAssignPtr>{makeSharedPtrToStack(hint)});
    Arrayi hintedResult = hinted.solve();
    EXPECT_TRUE(hintedResult.sliceTo(16).all([=](int i) {return i == 0;}));
    EXPECT_TRUE(hintedResult.sliceFrom(16).all([=](int i) {return i == 1;}));
  }{
    HintedStateAssign hinted(makeSharedPtrToStack(ref),
        Array<HintedStateAssign::LocalStateAssignPtr>
        {makeSharedPtrToStack(hint), makeSharedPtrToStack(hint2)});
    Arrayi hintedResult = hinted.solve();
    EXPECT_TRUE(hintedResult.sliceTo(16).all([=](int i) {return i == 0;}));
    EXPECT_TRUE(hintedResult.slice(16, 20).all([=](int i) {return i == 1;}));
    EXPECT_TRUE(hintedResult.sliceFrom(20).all([=](int i) {return i == 0;}));
  }
}

namespace {
class Ref2 : public StateAssign {
 public:
  Ref2() : _preds(listStateInds()) {}

  double getStateCost(int stateIndex, int timeIndex) {
    if (stateIndex == 0) {
      return 1.0 + sin(234.989*timeIndex);
    } else {
      return 1.0 + sin(324.34449*timeIndex + 2988.39964);
    }
  }

  double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {
    return std::abs(fromStateIndex - toStateIndex);
  }

  int getStateCount() {return 2;}

  int getLength() {return 30;}

  Arrayi getPrecedingStates(int stateIndex, int timeIndex) {
    return _preds;
  }
 private:
  Arrayi _preds;
};
}

TEST(HintedStateAssignTest, NoHintsTest) {
  Ref2 ref;
  HintedStateAssign hinted(makeSharedPtrToStack(ref), Array<HintedStateAssign::LocalStateAssignPtr>());
  EXPECT_EQ(ref.solve(), hinted.solve());
}


