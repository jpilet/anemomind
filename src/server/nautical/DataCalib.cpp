/*
 * DataCalib.cpp
 *
 *  Created on: 24 janv. 2014
 *      Author: jonas
 */

#include "DataCalib.h"
#include <server/nautical/LocalRace.h>

namespace sail {

DataCalib::DataCalib() {
  _paramCount = 0;
  _windDataCount = 0;
  _currentDataCount = 0;
  _navCount = 0;
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

arma::sp_mat DataCalib::makeP(Grid3d grid) {
  int count = _boats.size();
  const int elemsPerNav = 8; // 2^3, a linear combination of the 8 vertices of a cube
  int elemCount = _navCount*elemsPerNav;

  arma::umat IJ(2, elemCount);
  arma::vec X(elemCount);
  int offset = 0;
  for (int i = 0; i < count; i++) {
    _boats[i]->fillPData(offset, grid, &IJ, &X);
    offset += _boats[i]->getDataCount();
  }
  assert(offset == _navCount);
  return arma::sp_mat(IJ, X, _navCount, grid.getVertexCount());
}



BoatData::BoatData(LocalRace *race, Array<Nav> navs) : _race(race), _navs(navs), _paramOffset(0) {
}

void BoatData::fillPData(int navOffset, Grid3d grid, arma::umat *IJOut, arma::vec *XOut) {
  int count = _navs.size();
  const int PerNav = 8;
  for (int i = 0; i < count; i++) {
    arma::vec3 pos3 = _race->calcNavLocalPosAndTime(_navs[i]);

    int I[PerNav];
    double W[PerNav];
    int row = navOffset + i;
    int elemOffset = row*PerNav; // PerNav elements on every row
    grid.makeVertexLinearCombination(pos3.memptr(), I, W);
    for (int j = 0; j < PerNav; j++) {
      int at = elemOffset + j;
      (*IJOut)(0, at) = row;
      (*IJOut)(1, at) = I[j];
      (*XOut)[at] = W[j];
    }
  }
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
  _navCount += boatData->getDataCount();
}

} /* namespace sail */
