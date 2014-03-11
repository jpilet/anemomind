/*
 * DataCalib.cpp
 *
 *  Created on: 24 janv. 2014
 *      Author: jonas
 */

#include "DataCalib.h"
#include <server/nautical/LocalRace.h>
#include <server/common/math.h>
#include <server/common/string.h>

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
  assert(count > 0);
  assert(elemCount > 0);

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

arma::mat DataCalib::makeInitialParameters() {
  int count = _boats.size();
  int paramCount = 0;
  for (int i = 0; i < count; i++) {
    paramCount += _boats[i]->paramCount();
  }

  arma::mat X(paramCount, 1);
  for (int i = 0; i < count; i++) {
    _boats[i]->initializeParameters(X.memptr());
  }
  return X;
}


BoatData::BoatData(LocalRace *race, Array<Nav> navs, DriftModel *driftModel) :
    _race(race), _navs(navs), _paramOffset(0), _driftModel(driftModel) {
}

void BoatData::evalCurrentData(adouble *Xin, adouble *Fout) {
  int navCount = _navs.size();
  for (int i = 0; i < navCount; i++) {
    adouble *waterWrtEarth = Fout + 2*i;
    Nav &nav = _navs[i];
    adouble beta = estimateHeadingRadians(nav, Xin);
    adouble apparentWaterSpeed = calcWaterSpeedMPS(nav, Xin);
    arma::advec2 boatWrtWater = apparentWaterSpeed*_race->calcNavLocalDir(nav, beta);
    arma::advec2 boatWrtEarth = calcBoatWrtEarth(nav);
    for (int j = 0; j < 2; j++) {
      waterWrtEarth[j] = boatWrtEarth[j] - boatWrtWater[j]; // boatWrtWater + waterWrtEarth = boatWrtEarth
    }
  }
}

adouble BoatData::estimateHeadingRadians(const Nav &nav, adouble *Xin) {
  adouble magHdg = magneticCompassOffset(Xin) +
        _race->getMagDecl() +
        nav.magHdg().radians();

  return _driftModel->calcCorrectionAngle(this, nav, Xin) + magHdg;
}

arma::advec2 BoatData::calcBoatWrtEarth(const Nav &nav) {
  return nav.gpsSpeed().metersPerSecond()*_race->calcNavLocalDir(nav, nav.gpsBearing().radians());
}


adouble BoatData::calcAwaRadians(const Nav &nav, adouble *Xin) {
  return nav.awa().radians() + windDirectionOffset(Xin);
}

adouble BoatData::calcAwsMPS(const Nav &nav, adouble *Xin) {
  return nav.aws().metersPerSecond()*windSpeedCoef(Xin);
}

adouble BoatData::calcWaterSpeedMPS(const Nav &nav, adouble *Xin) {
  return waterSpeedCoef(Xin)*nav.watSpeed().metersPerSecond();
}



void BoatData::evalWindData(adouble *Xin, adouble *Fout) {
  int navCount = _navs.size();
  for (int i = 0; i < navCount; i++) {
    adouble *windWrtEarth = Fout + 2*i;
    Nav &nav = _navs[i];
    adouble awsMPS = windSpeedCoef(Xin)*nav.aws().metersPerSecond();
    adouble awaRadians = calcAwaRadians(nav, Xin);
    arma::advec2 windWrtBoat = awsMPS*_race->calcNavLocalDir(nav, awaRadians);
    arma::advec2 boatWrtEarth = calcBoatWrtEarth(nav);
    for (int j = 0; j < 2; j++) {
      windWrtEarth[j] = boatWrtEarth[j] - windWrtBoat[j];
    }
  }
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

void BoatData::initializeParameters(double *XOut) {
  magneticCompassOffset(XOut) = 0.0;
  windDirectionOffset(XOut) = 0.0;
  waterSpeedCoef(XOut) = 1.0;
  windSpeedCoef(XOut) = 1.0;
  _driftModel->initializeParameters(XOut + driftModelParamOffset());
}







void DataCalib::addBoatData(std::shared_ptr<BoatData> boatData) {
  boatData->setParamOffset(_paramCount);
  _boats.push_back(boatData);
  _paramCount += boatData->paramCount();
  _windDataCount += boatData->getWindDataCount();
  _currentDataCount += boatData->getCurrentDataCount();
  _navCount += boatData->getDataCount();
}

} /* namespace sail */
