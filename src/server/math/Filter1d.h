/*
 *  Created on: 2014-10-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef FILTER1D_H_
#define FILTER1D_H_

namespace sail {

Arrayd filter1d(LineStrip strip, Arrayd X, Arrayd Y,
    Array<Arrayb> crossValidationSplits,
    LevmarSettings s = LevmarSettings());


Arrayd filter1d(Arrayd X,
    Array<Arrayb> crossValidationSplits,
    LevmarSettings s = LevmarSettings());

}

#endif /* FILTER1D_H_ */
