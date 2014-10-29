/*
 *  Created on: 2014-10-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef GENERALIZEDTVAUTO_H_
#define GENERALIZEDTVAUTO_H_

#include <server/math/nonlinear/GeneralizedTV.h>

namespace sail {

/*
 * Performs the same job as GernalizedTV, but automatically
 * figures out the regularization weight using cross-validation.
 *
 * Using this class to filter a signal takes longer, because
 * it needs to filter the signal for many different values of
 * a regularization weight.
 *
 */
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
