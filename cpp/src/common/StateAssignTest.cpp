/*
 * StateAssignTest.cpp
 *
 *  Created on: 24 janv. 2014
 *      Author: jonas
 */

#include "gtest/gtest.h"
#include "StateAssign.h"

using namespace sail;

class SATestNoTransitionCost : public StateAssign
{
public:
	SATestNoTransitionCost();
	double getStateCost(int stateIndex, int timeIndex);
	double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {return 0;}
	int getStateCount() {return 2;}
	int getLength() {return 12;}
	Arrayi getPrecedingStates(int stateIndex, int timeIndex) {return _pred;}
private:
	Arrayi _pred;
};

double SATestNoTransitionCost::getStateCost(int stateIndex, int timeIndex)
{
	if (stateIndex == 0)
	{
		return 0.1;
	}
	else
	{
		int local = (timeIndex % 4) - 2;
		return local;
	}
}

SATestNoTransitionCost::SATestNoTransitionCost() : _pred(listStateInds())
{
}

TEST(StateAssignTest, NoTransitionCost)
{
	SATestNoTransitionCost test;
	Arrayi result = test.solve();
	EXPECT_TRUE(result.size() == test.getLength());
	for (int i = 0; i < test.getLength(); i++)
	{
		EXPECT_TRUE((result[i] == 0) == (test.getStateCost(0, i) < test.getStateCost(1, i)));
	}
}


namespace
{
	char toChar(int num)
	{
		const char chars[] = "0123456789";
		return chars[num];
	}
}


class SATestNoisyStep : public StateAssign
{
public:
	SATestNoisyStep(std::string noisy);
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

SATestNoisyStep::SATestNoisyStep(std::string noisy) : _noisy(noisy), _transitionCost(4.0)
{
	_preds.create(2);
	_preds[0] = listStateInds();
	_preds[1] = listStateInds();
}

double SATestNoisyStep::getStateCost(int stateIndex, int timeIndex)
{
	return (toChar(stateIndex) == _noisy[timeIndex]? 0.0 : 1.0);
}

double SATestNoisyStep::getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex)
{
	return (fromStateIndex == toStateIndex? 0.0 : _transitionCost);
}

int SATestNoisyStep::getStateCount() {return 2;}

int SATestNoisyStep::getLength() {return _noisy.length();}

Arrayi SATestNoisyStep::getPrecedingStates(int stateIndex, int timeIndex)
{
	return _preds[stateIndex];
}


void SATestNoisyStep::useGrammar()
{
	_transitionCost = 0.0;
	_preds[0] = Arrayi::args(0);
}

TEST(StateAssignTest, NoisyStep)
{
	std::string noisy = "000010010100011111110111001111";
	std::string gt    = "000000000000011111111111111111";

	SATestNoisyStep test(noisy);
	Arrayi result = test.solve();
	EXPECT_TRUE(result.size() == gt.length());
	for (int i = 0; i < result.size(); i++)
	{
		EXPECT_TRUE(toChar(result[i]) == gt[i]);
	}
}

TEST(StateAssignTest, NoisyStepGrammar)
{
	std::string noisy = "000010010100011111110111001111";
	std::string gt    = "000000000000011111111111111111";

	SATestNoisyStep test(noisy);
	test.useGrammar();

	Arrayi result = test.solve();


	EXPECT_TRUE(result.size() == gt.length());
	for (int i = 0; i < result.size(); i++)
	{
		EXPECT_TRUE(toChar(result[i]) == gt[i]);
	}
}


