/*
 * LevMar.h
 *
 *  Created on: 21 janv. 2014
 *      Author: jonas
 */

#ifndef LEVMAR_H_
#define LEVMAR_H_

#include <server/common/Function.h>
#include <server/common/Array.h>
#include <armadillo>

namespace sail {

class LevmarSettings;
class LevmarState {
 public:
  LevmarState(arma::mat X);
  LevmarState(Arrayd X);

  // Take a single step
  void step(const LevmarSettings &settings, Function &fun);

  void minimize(const LevmarSettings &settings, Function &fun);

  arma::mat &getX() {
    return _X;
  }

  Arrayd getXArray() {return Arrayd(_X.n_elem, _X.memptr());}
 private:
  void initialize(arma::mat X);
  arma::mat _X;
  double _v, _mu;
  arma::mat JtJ, JtF;
  bool _stop;

  arma::mat _Fscratch, _Jscratch;
};

} /* namespace sail */

#endif /* LEVMAR_H_ */
