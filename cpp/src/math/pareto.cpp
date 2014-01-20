/*
 * pareto.cpp
 *
 *  Created on: 18 janv. 2014
 *      Author: jonas
 */

#include "pareto.h"
#include <assert.h>
#include "../common/common.h"

namespace sail
{


ParetoElement::ParetoElement()
{
	_len = 0;
}

ParetoElement::ParetoElement(double a)
{
	_len = 1;
	_values[0] = a;
}

ParetoElement::ParetoElement(double a, double b)
{
	_len = 2;
	_values[0] = a;
	_values[1] = b;
}

ParetoElement::ParetoElement(Arrayd arr)
{
	_len = arr.size();
	assert(_len <= MAXPARETOLEN);
	for (int i = 0; i < _len; i++)
	{
		_values[i] = arr[i];
	}
}

ParetoElement::ParetoElement(int len, double *arr)
{
	_len = len;
	assert(_len <= MAXPARETOLEN);
	for (int i = 0; i < _len; i++)
	{
		_values[i] = arr[i];
	}
}

bool ParetoElement::dominates(const ParetoElement &other) const
{
	assert(_len == other._len);
	for (int i = 0; i < _len; i++)
	{
		if (!(_values[i] < other._values[i]))
		{
			return false;
		}
	}
	return true;
}

bool ParetoFrontier::accepts(const ParetoElement &e)
{
	int count = _frontier.size();
	for (int i = 0; i < count; i++)
	{
		if (_frontier[i].dominates(e))
		{
			return false;
		}
	}
	return true;
}

bool ParetoFrontier::insert(const ParetoElement &e)
{
	assert(e.initialized());
	int count = _frontier.size();
	Arrayb keep(count);
	for (int i = 0; i < count; i++)
	{
		assert(_frontier[i].initialized());
		if (_frontier[i].dominates(e))
		{
			return false;
		}
		keep[i] = !e.dominates(_frontier[i]);
	}

	int counter = 0;
	for (int i = 0; i < count; i++)
	{
		if (keep[i])
		{
			_frontier[counter] = _frontier[i];
			counter++;
		}
	}
	_frontier.resize(counter + 1);
	_frontier[counter] = e;

	return true;
}


} /* namespace sail */
