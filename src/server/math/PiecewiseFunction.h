/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef PIECEWISEFUNCTION_H_
#define PIECEWISEFUNCTION_H_

#include <functional>
#include <server/common/ProportionateIndexer.h>
#include <server/common/LineKM.h>

namespace sail {

class PiecewiseFunction {
 public:
  typedef LineKM Fun;
  class Piece {
   public:
    Piece() : width(0) {}
    Piece(double w, Fun f) :
      width(w), fun(f) {}

    double width;
    Fun fun;
  };

  PiecewiseFunction(double left, Array<Piece> pieces);
  double operator() (double x) const;
  std::function<double(double)> make() const;
 private:
  double _left;
  ProportionateIndexer _indexer;
  Array<Piece> _pieces;
};



}
#endif /* PIECEWISEFUNCTION_H_ */
