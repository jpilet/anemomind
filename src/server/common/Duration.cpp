/*
 * Duration.cpp
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#include "Duration.h"
#include <cmath>
#include <sstream>

namespace sail
{

Duration::Duration(double seconds)
{
	_seconds = fmod(seconds, 60.0);
	int minutes = int(floor(seconds/60.0));
	_minutes = minutes % 60;
	int hours = minutes / 60;
	_hours = hours % 24;
	int days = hours / 24;
	_days = days % 7;
	_weeks = days / 7;
}

Duration::Duration()
{
	_weeks = 0;
	_days = 0;
	_hours = 0;
	_minutes = 0;
	_seconds = 0;
}


Duration::Duration(unsigned int weeks, unsigned int days, unsigned int hours,
		unsigned int minutes, double seconds)
{
	_weeks = weeks;
	_days = days;
	_hours = hours;
	_minutes = minutes;
	_seconds = seconds;
}


Duration::~Duration()
{
	// TODO Auto-generated destructor stub
}

Duration Duration::minutes(unsigned int minutes)
{
	return Duration(0, 0, 0, minutes, 0.0);
}

#define DISPDUR(label, val) if ((val) > 0) { if (flag) {ss << ", ";} ss << val << " " << (label); flag = true;}

std::string Duration::str()
{
	std::stringstream ss;
	bool flag = false;
	DISPDUR("weeks", _weeks);
	DISPDUR("days", _days);
	DISPDUR("hours", _hours);
	DISPDUR("minutes", _minutes);
	DISPDUR("seconds", _seconds);
	return ss.str();
}

double Duration::getDurationSeconds()
{
	return _seconds + 60*(_minutes + 60*(_hours + 24*(_days + 7*_weeks)));
}

} /* namespace sail */
