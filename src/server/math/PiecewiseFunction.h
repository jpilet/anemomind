/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef PIECEWISEFUNCTION_H_
#define PIECEWISEFUNCTION_H_

#include <functional>
#include <server/common/ProportionateIndexer.h>

namespace sail {

template <typename T>
class PiecewiseFunction {
 public:
  typedef std::function<T(double)> Fun;
  class Piece {
   public:
    Piece() : width(0) {}
    Piece(Fun f, double w) :
      width(w), fun(f) {}

    double width;
    Fun fun;
  };

  PiecewiseFunction(Array<Piece> pieces) :
    _pieces(pieces) {
    _indexer = ProportionateIndexer(pieces.size(),
        [&](int index) {return pieces[index].width;});
  }

  T operator() (double x) const {
    return _pieces[_indexer.getBySum(x).index].fun(x);
  }

  Fun make() const {
    PiecewiseFunction fun = *this;
    return [=](double x) {
      return fun(x);
    };
  }
 private:
  ProportionateIndexer _indexer;
  Array<Piece> _pieces;
};



}
#endif /* PIECEWISEFUNCTION_H_ */
