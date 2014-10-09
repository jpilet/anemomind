/*
 *  Created on: 2014-10-09
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef GENARALIZEDTV_H_
#define GENARALIZEDTV_H_

namespace sail {

class GeneralizedTV {
 public:
  GeneralizedTV(int iters, double minv) :
    _iters(iters), _minv(minv) {}

  Arrayd filter(LineStrip strip,
                Arrayd initVertices,
                Arrayd X, Arrayd Y);
  Arrayd filter(Arrayd );
 private:
  // Settings
  int _iters;
  double _minv;
};



}

#endif /* GENARALIZEDTV_H_ */
