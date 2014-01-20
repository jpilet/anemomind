/*
 * pareto.h
 *
 *  Created on: 18 janv. 2014
 *      Author: jonas
 */

#ifndef PARETO_H_
#define PARETO_H_

#include "../common/Array.h"
#include <vector>

namespace sail
{

#define MAXPARETOLEN 12

class ParetoElement
{
public:
	ParetoElement();
	bool initialized() const {return _len > 0;}
	ParetoElement(double a);
	ParetoElement(double a, double b);
	ParetoElement(Arrayd arr);
	ParetoElement(int len, double *arr);

	bool dominates(const ParetoElement &other) const;

	virtual ~ParetoElement() {}

	int getLength() {return _len;}
	double getValue(int index) {return _values[index];}
private:
	int _len;
	double _values[MAXPARETOLEN];
};

class ParetoFrontier
{
public:
	bool accepts(const ParetoElement &e);
	bool insert(const ParetoElement &e);
	int size() {return _frontier.size();}
private:
	std::vector<ParetoElement> _frontier;
};

} /* namespace sail */

#endif /* PARETO_H_ */
