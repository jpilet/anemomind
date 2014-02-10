/*
 * DataCalib.cpp
 *
 *  Created on: 24 janv. 2014
 *      Author: jonas
 */

#include "DataCalib.h"

namespace sail
{

DataCalib::DataCalib()
{
	_paramCount = 0;
}

DataCalib::~DataCalib()
{

}

BoatData::BoatData(LocalRace *race, Array<Nav> navs) : _race(race), _navs(navs), _paramOffset(0)
{
}

void BoatData::setParamOffset(int offset)
{
	_paramOffset = offset;
}


void DataCalib::addBoatData(std::shared_ptr<BoatData> boatData)
{
	boatData->setParamOffset(_paramCount);
	_boats.push_back(boatData);
	_paramCount += boatData->getParamCount();
}

} /* namespace sail */
