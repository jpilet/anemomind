/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef SERVER_NAUTICAL_NAVCOMPATIBILITY_H_
#define SERVER_NAUTICAL_NAVCOMPATIBILITY_H_

#include <server/nautical/Nav.h>
#include <server/nautical/NavDataset.h>


namespace sail {

namespace NavCompat {

int getNavSize(const NavDataset &ds);
bool isValidNavIndex(const NavDataset &ds, int i);
bool isValidNavBoundaryIndex(const NavDataset &ds, int i);
const Nav getNav(const NavDataset &ds, int i);
NavDataset slice(const NavDataset &ds, int from, int to);
NavDataset sliceFrom(const NavDataset &ds, int index);
NavDataset sliceTo(const NavDataset &ds, int index);
const Nav getLast(const NavDataset &ds);
const Nav getFirst(const NavDataset &ds);
int getMiddleIndex(const NavDataset &ds);
int getLastIndex(const NavDataset &ds);
bool isEmpty(const NavDataset &ds);
Array<Nav> makeArray(const NavDataset &ds);
NavDataset fromNavs(const Array<Nav> &navs);

class Iterator : public std::iterator<random_access_iterator_tag, Nav> {
 public:
  Iterator(const NavDataset &coll, int index) :
    _coll(coll), _index(index) {}

  const Nav operator*() const {
    return getNav(_coll, _index);
  }

  Iterator& operator++() {
    _index++;
    return *this;
  }

  bool operator==(const Iterator &other) const {
    return _index == other._index && _coll == other._coll;
  }

  bool operator!=(const Iterator &other) const {
    return _index != other._index;
  }

  Iterator operator+(int x) const {
    return Iterator(_coll, _index + x);
  }

  int operator-(const Iterator &other) const {
    return _index - other._index;
  }
 private:
  NavDataset _coll;
  int _index;
};

Iterator getBegin(const NavDataset &ds);
Iterator getEnd(const NavDataset &ds);


class Range {
 public:
  Range(const NavDataset &ds) : _begin(getBegin(ds)), _end(getEnd(ds)) {}

  Iterator begin() const {return _begin;}
  Iterator end() const {return _end;}

  int size() const {
    return _end - _begin;
  }

  Nav operator[](int index) const {
    return *(_begin + index);
  }

 private:
  Iterator _begin, _end;
};

}

NavDataset loadNavsFromText(std::string filename, bool sort = true);
bool areSortedNavs(NavDataset navs);
void plotNavTimeVsIndex(NavDataset navs);
void dispNavTimeIntervals(NavDataset navs);
Array<NavDataset> splitNavsByDuration(NavDataset navs, Duration<double> dur);
MDArray2d calcNavsEcefTrajectory(NavDataset navs);
Array<MDArray2d> calcNavsEcefTrajectories(Array<NavDataset > navs);
void plotNavsEcefTrajectory(NavDataset navs);
void plotNavsEcefTrajectories(Array<NavDataset> navs);
int countNavs(Array<NavDataset> navs);
Length<double> computeTrajectoryLength(NavDataset navs);
int findMaxSpeedOverGround(NavDataset navs);

}

#endif /* SERVER_NAUTICAL_NAVCOMPATIBILITY_H_ */
