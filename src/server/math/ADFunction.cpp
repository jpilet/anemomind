/*
 * ADFunction.cpp
 *
 *  Created on: 20 janv. 2014
 *      Author: jonas
 */

#include "ADFunction.h"
#include <server/math/armaadolc.h>
#include <adolc/taping.h>

namespace sail {

void AutoDiffFunction::eval(double *Xin, double *Fout, double *Jout) {
  int indims = inDims();
  int outdims = outDims();
  bool outputJ = Jout != nullptr;

  short int tag = getTapeTag();

  if (outputJ) {
    trace_on(tag);
  }


  Arrayad adX(indims);
  adolcInput(indims, adX.getData(), Xin);

  Arrayad adF(outdims);
  evalAD(adX.getData(), adF.getData());

  adolcOutput(outdims, adF.getData(), Fout);


  if (outputJ) {
    trace_off();
    outputJacobianColMajor(tag, Xin, Jout);
  }
}

} /* namespace sail */
