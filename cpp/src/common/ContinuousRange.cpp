/*
 * SpacialRange.cpp
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#include "ContinuousRange.h"
#include <cmath>
#include <assert.h>

namespace sail
{

ContinuousRange::ContinuousRange()
{
	_periodic = false;
	_mean = 0;
	_maxDif = 0;
}

ContinuousRange::ContinuousRange(Arrayd values, bool periodic) : _periodic(periodic)
{
	int count = values.size();
	double f = 1.0/count;
	if (periodic)
	{
		double x = 0.0;
		double y = 0.0;
		for (int i = 0; i < count; i++)
		{
			double angle = values[i];
			x += cos(angle);
			y += sin(angle);
		}
		_mean = atan2(y, x);
		_maxDif = 0.0;
		for (int i = 0; i < count; i++)
		{
			double angle = values[i];
			_maxDif = std::max(_maxDif, std::acos(std::cos(angle - _mean)));
		}
		assert(_maxDif <= M_PI);
	}
	else
	{
		double sum = 0;
		for (int i = 0; i < count; i++)
		{
			sum += values[i];
		}
		_mean = f*sum;
		_maxDif = 0;
		for (int i = 0; i < count; i++)
		{
			_maxDif = std::max(_maxDif, std::abs(values[i] - _mean));
		}
	}
	assert(_maxDif > 0);
}

bool ContinuousRange::intersects(const ContinuousRange &other) const
{
	assert(_periodic == other._periodic);
	if (_periodic)
	{
		// Assuming this object has the greater interval. Does it contain any part of the other?
		if (_maxDif >= other._maxDif)
		{
			double thresh = cos(_maxDif);
			double offs = other._mean - _mean;
			return (cos(offs) >= thresh) ||
						(cos(offs - other._maxDif) >= thresh) ||
						(cos(offs + other._maxDif) >= thresh);
		}
		else
		{
			return other.intersects(*this);
		}
	}
	else
	{
		double a = _mean;
		double ad = _maxDif;
		double b = other._mean;
		double bd = other._maxDif;
		if (_mean > other._mean)
		{
			std::swap(a, b);
			std::swap(ad, bd);
		}
		return (a + ad) > (b - bd);
	}
}


} /* namespace sail */
