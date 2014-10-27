/*
 *  Created on: 2014-10-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef GENERALIZEDTVAUTO_H_
#define GENERALIZEDTVAUTO_H_

#include <server/math/nonlinear/GeneralizedTV.h>

namespace sail {

class GeneralizedTVAuto {
 public:
  GeneralizedTVAuto(const GeneralizedTV &tv_,
    double initX = 1.0, double step = 1.0, int maxIter = 4);

  const GeneralizedTV &tv() const {
    return _tv;
  }

  double optimizeRegWeight(UniformSamplesd initialSignal,
                  Arrayd X, Arrayd Y,
                  int order,
                  Array<Arrayb> splits = Array<Arrayb>()) const;

  UniformSamplesd filter(UniformSamplesd initialSignal,
                  Arrayd X, Arrayd Y,
                  int order,
                  Array<Arrayb> initSplits = Array<Arrayb>()) const;
  UniformSamplesd filter(Arrayd Y, int order, Array<Arrayb> splits = Array<Arrayb>()) const;
 private:
  GeneralizedTV _tv;
  double _initX, _step;
  int _maxIter;
  double _factor;
};

}

#endif /* GENERALIZEDTVAUTO_H_ */
