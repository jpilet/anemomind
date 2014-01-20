/*
 * SpacialRange.h
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#ifndef SPACIALRANGE_H_
#define SPACIALRANGE_H_

#include "Array.h"

namespace sail
{

class ContinuousRange
{
public:
	ContinuousRange();
	ContinuousRange(Arrayd values, bool periodic);
	virtual ~ContinuousRange() {}
	bool intersects(const ContinuousRange &other) const;
private:
	double _mean, _maxDif;
	bool _periodic;
};

typedef ContinuousRange CRange;

} /* namespace sail */

#endif /* SPACIALRANGE_H_ */
