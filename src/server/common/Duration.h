/*
 * Duration.h
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#ifndef DURATION_H_
#define DURATION_H_

#include <string>

namespace sail
{

// Conveniency class for displaying durations
// and converting seconds back and forth.
class Duration
{
public:
	Duration(double seconds);
	Duration();
	Duration(unsigned int weeks, unsigned int days, unsigned int hours,
			unsigned int minutes, double seconds);

	static Duration minutes(unsigned int minutes);


	virtual ~Duration();

	std::string str();

	double getDurationSeconds();
private:
	double _seconds;
	unsigned int _minutes, _hours, _days, _weeks;
};

} /* namespace sail */

#endif /* DURATION_H_ */
