/*
 * LocalRace.cpp
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#include "LocalRace.h"
#include <server/common/ArrayIO.h>
#include <server/plot/extra.h>
#include <server/nautical/WGS84.h>

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
  //nav.get3dPos(pos3d.memptr());
  Length<double> xyz3[3];
  double xyz3m[3];
  WGS84<double>::toXYZ(nav.geographicPosition(), xyz3);
  for (int i = 0; i < 3; i++) {
    xyz3m[i] = xyz3[i].meters();
  }
  return _axes*(pos3d - _cog);
}

double LocalRace::calcNavLocalTime(const Nav &nav) {
  return nav.time().seconds() - _timeOffset;
}

arma::vec3 LocalRace::calcNavLocalPosAndTime(Nav nav) {
  arma::vec2 xy = calcNavLocalPos(nav);
  double data[3] = {xy[0], xy[1], calcNavLocalTime(nav)};
  return arma::vec3(data);
}

arma::Col<adouble>::fixed<2> LocalRace::calcNavLocalDir(Nav nav, adouble dirRadians) {
  adouble xyzDir[3];
  GeographicPosition<adouble> pos = GeographicPosition<adouble>(nav.geographicPosition());
  Length<adouble> xyz[3];
  WGS84<adouble>::posAndDirToXYZ(pos, Angle<adouble>::radians(dirRadians),
      xyz, xyzDir);
  arma::Col<adouble>::fixed<2> dst;
  dst[0] = 0;
  dst[1] = 0;
  for (int i = 0; i < 3; i++) {
    dst[0] += _axes(0, i)*xyzDir[i];
    dst[1] += _axes(1, i)*xyzDir[i];
  }
  normalizeInPlace<adouble>(2, dst.memptr());
  return dst;
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
    //navs[i].get3dPos(&(pos(i, 0)), &(pos(i, 1)), &(pos(i, 2)));
    const GeographicPosition<double> &gpos = navs[i].geographicPosition();
    Length<double> xyz[3];
    WGS84<double>::toXYZ(gpos, xyz);
    for (int j = 0; j < 3; j++) {
      pos(i, j) = xyz[j].meters();
    }
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

void LocalRace::plotTrajectoryVectors(Array<Nav> navs, Arrayd vectors2d, double scale) {
  int count = navs.size();
  assert(2*count == vectors2d.size());
  MDArray2d localPos3d = calcNavsLocalPosAndTime(navs);
  MDArray2d vertices = _wind.getGridVertexCoords();

  GnuplotExtra plot;
  plot.set_style("points");
  plot.plot(vertices);
  plot.set_style("lines");
  plot.plot(calcNavsLocalPosAndTime(navs));
  for (int i = 0; i < count; i++) {
    double *v = vectors2d.blockPtr(i, 2);
    double from[3] = {localPos3d(i, 0), localPos3d(i, 1), localPos3d(i, 2)};
    double   to[3] = {from[0] + scale*v[0], from[1] + scale*v[1], from[2]};
    plot.plot(3, from, to);
  }

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
    double vec[3] = {xy[0], xy[1], nav.time().seconds()};
    result.extend(vec);
  }
  return result;
}


} /* namespace sail */
