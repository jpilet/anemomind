/*
 * LocalRace.cpp
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#include "LocalRace.h"
#include <server/common/ArrayIO.h>
#include <server/plot/extra.h>

namespace sail {

LocalRace::LocalRace() {
  // TODO Auto-generated constructor stub
  _timeOffset = 0;
  _magDeclRadians = 0.0;
}

LocalRace::~LocalRace() {
  // TODO Auto-generated destructor stub
}

LocalRace::LocalRace(Array<Array<Nav> > navs, double spaceStep, double timeStep) {
  initialize(navs.join(), spaceStep, timeStep);
}

LocalRace::LocalRace(Array<Nav> navs, double spaceStep, double timeStep) {
  initialize(navs, spaceStep, timeStep);
}

arma::vec2 LocalRace::calcNavLocalPos(Nav nav) {
  arma::vec3 pos3d;
  nav.getEcef3dPos(pos3d[0], pos3d[1], pos3d[2]);
  return _axes*(pos3d - _cog);
}

double LocalRace::calcNavLocalTime(const Nav &nav) {
  return nav.getTimeSeconds() - _timeOffset;
}

arma::vec3 LocalRace::calcNavLocalPosAndTime(Nav nav) {
  arma::vec2 xy = calcNavLocalPos(nav);
  double data[3] = {xy[0], xy[1], calcNavLocalTime(nav)};
  return arma::vec3(data);
}

MDArray2d LocalRace::calcNavsLocalPosAndTime(Array<Nav> navs) {
  int count = navs.size();
  MDArray2d data(count, 3);
  for (int i = 0; i < count; i++) {
    arma::vec3 x = calcNavLocalPosAndTime(navs[i]);
    for (int j = 0; j < 3; j++) {
      data(i, j) = x[j];
    }
  }
  return data;
}


Array<arma::vec2> LocalRace::calcNavsLocalPos(Array<Nav> navs) {
  return navs.map<arma::vec2>([&](Nav x) {
    return calcNavLocalPos(x);
  });
}


arma::mat getAllNav3dPos(Array<Nav> navs) {
  int count = navs.size();
  arma::mat pos(count, 3);
  for (int i = 0; i < count; i++) {
    navs[i].getEcef3dPos(pos(i, 0), pos(i, 1), pos(i, 2));
  }
  return pos;
}

void initializeLocalRaceCoord(arma::mat pos, arma::vec3 &cogOut, arma::mat23 &axesOut) {
  int n = pos.n_rows;
  cogOut = arma::mean(pos, 0).t();
  arma::mat centred = pos - arma::repmat(cogOut.t(), n, 1);
  arma::mat33 sigma = cov(centred);

  arma::vec eigval;
  arma::mat eigvec;
  arma::eig_sym(eigval, eigvec, sigma);

  cout << "Standard deviation of error in altitude due to planar approximation: "
       << sqrt(eigval[0]) << " metres." << endl;
  axesOut = eigvec.cols(1, 2).t();
}


void LocalRace::initialize(Array<Nav> navs, double spaceStep, double timeStep) {
  arma::mat pos = getAllNav3dPos(navs);
  initializeLocalRaceCoord(pos, _cog, _axes);

  BBox3d bbox = calcBBoxXYTimeWithoutOffsetDuringInitialization(navs);
  _timeOffset = bbox.getSpan(2).subtractMean();
  double gridSpacing[3] = {spaceStep, spaceStep, timeStep};
  _wind = Grid3d(bbox, gridSpacing);
  _current = Grid3d(bbox, gridSpacing);
}

void LocalRace::makeSpatioTemporalPlot(Array<Nav> navs) {
  MDArray2d vertices = _wind.getGridVertexCoords();
  GnuplotExtra plot;
  plot.set_style("points");
  plot.plot(vertices);
  plot.set_style("lines");
  plot.plot(calcNavsLocalPosAndTime(navs));
  plot.show();
}

Grid3d &LocalRace::getWindGrid() {
  return _wind;
}

Grid3d &LocalRace::getCurrentGrid() {
  return _current;
}

void LocalRace::setMagDecl(double magDeclRadians) {
  _magDeclRadians = magDeclRadians;
}


BBox3d LocalRace::calcBBoxXYTimeWithoutOffsetDuringInitialization(Array<Nav> navs) {
  BBox3d result;
  int count = navs.size();
  for (int i = 0; i < count; i++) {
    Nav &nav = navs[i];
    arma::vec2 xy = calcNavLocalPos(nav);
    double vec[3] = {xy[0], xy[1], nav.getTimeSeconds()};
    result.extend(vec);
  }
  return result;
}


} /* namespace sail */
