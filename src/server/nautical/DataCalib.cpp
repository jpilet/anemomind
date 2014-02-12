/*
 * DataCalib.cpp
 *
 *  Created on: 24 janv. 2014
 *      Author: jonas
 */

#include "DataCalib.h"

namespace sail {

DataCalib::DataCalib() {
  _paramCount = 0;
  _windDataCount = 0;
  _currentDataCount = 0;
}

DataCalib::~DataCalib() {

}

void DataCalib::evalWindData(adouble *Xin, adouble *Fout) {
  int offset = 0;
  int count = _boats.size();
  for (int i = 0; i < count; i++) {
    BoatData *boat = _boats[i].get();
    boat->evalWindData(Xin, Fout + offset);
    offset += boat->getWindDataCount();
  }
  assert(offset == _windDataCount);
}

void DataCalib::evalCurrentData(adouble *Xin, adouble *Fout) {
  int offset = 0;
  int count = _boats.size();
  for (int i = 0; i < count; i++) {
    BoatData *boat = _boats[i].get();
    boat->evalCurrentData(Xin, Fout + offset);
    offset += boat->getCurrentDataCount();
  }
  assert(offset == _currentDataCount);
}


BoatData::BoatData(LocalRace *race, Array<Nav> navs) : _race(race), _navs(navs), _paramOffset(0) {
}

void BoatData::setParamOffset(int offset) {
  _paramOffset = offset;
}


void DataCalib::addBoatData(std::shared_ptr<BoatData> boatData) {
  boatData->setParamOffset(_paramCount);
  _boats.push_back(boatData);
  _paramCount += boatData->getParamCount();
  _windDataCount += boatData->getWindDataCount();
  _currentDataCount += boatData->getCurrentDataCount();
}

} /* namespace sail */
