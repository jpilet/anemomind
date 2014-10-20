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
    double initValue = 1.0, double step = 1.0, int maxIter = 4);

  const GeneralizedTV &tv() const {
    return _tv;
  }

  UniformSamples filter(UniformSamples initialSignal,
                  Arrayd X, Arrayd Y,
                  int order,
                  Array<Arrayb> splits = Array<Arrayb>()) const;

  UniformSamples filter(Arrayd Y, int order, Array<Arrayb> splits = Array<Arrayb>()) const;
 private:
  GeneralizedTV _tv;
  double _initValue, _step;
  int _maxIter;
  const double _factor;
};

}

#endif /* GENERALIZEDTVAUTO_H_ */
