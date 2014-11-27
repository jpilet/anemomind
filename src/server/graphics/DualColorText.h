/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef DUALCOLORTEXT_H_
#define DUALCOLORTEXT_H_

namespace sail {

class DualColorText : public R2ImageRGB {
 public:
  DualColorText(BitMapText bmt,
      R2ImageRGB::Vec bg,
      R2ImageRGB::Vec fg1,
      R2ImageRGB::Vec fg2,
      double frac) : _bmt(bmt),
      _bg(bg), _fg1(fg1), _fg2(fg2),
      _split(frac*bmt.width()) {}

  Vec operator() (double x, double y) const {
    bool v = _bmt(x, y);
    if (v) {
      return (x < _split? _fg1 : _fg2);
    }
    return _bg;
  }

  double width() const {return _bmt.width();}
  double height() const {return _bmt.height();}
 private:
  BitMapText _bmt;
  R2ImageRGB::Vec _bg;
  R2ImageRGB::Vec _fg1;
  R2ImageRGB::Vec _fg2;
  double _split;
};

}

#endif /* DUALCOLORTEXT_H_ */
