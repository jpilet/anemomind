/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/PiecewiseFunction.h>

namespace sail {


PiecewiseFunction::PiecewiseFunction(double left, Array<Piece> pieces) :
  _pieces(pieces),
  _left(left) {
  _indexer = ProportionateIndexer(pieces.size(),
      [=](int index) {return pieces[index].width;});
}

double PiecewiseFunction::operator() (double x) const {
  return _pieces[_indexer.getBySum(x - _left).index].fun(x);
}

std::function<double(double)> PiecewiseFunction::make() const {
  PiecewiseFunction fun = *this;
  return [=](double x) {
    return fun(x);
  };
}



}



