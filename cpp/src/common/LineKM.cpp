/*
 * LineKM.cpp
 *
 *  Created on: 16 janv. 2014
 *      Author: jonas
 */

#include "LineKM.h"
#include "common.h"

namespace sail
{


LineKM::LineKM(double x0, double x1, double y0, double y1)
{
	calcLineKM(x0, x1, y0, y1, _k, _m);
}

LineKM::LineKM()
{
	_k = 1.0;
	_m = 0.0;
}

double LineKM::operator() (double x) const
{
	return _k*x + _m;
}

double LineKM::inv(double x) const
{
	return (x - _m)/_k;
}

double LineKM::getK() const
{
	return _k;
}

double LineKM::getM() const
{
	return _m;
}


LineKM::~LineKM()
{}

std::ostream &operator<<(std::ostream &s, LineKM line)
{
	s << "Line y = " << line.getK() << "*x + " << line.getM();
	return s;
}

} /* namespace sail */
