/*
 * LsqNashSolver.cpp
 *
 *  Created on: 6 Jan 2017
 *      Author: jonas
 */

#include "LsqNashSolver.h"

namespace sail {
namespace LsqNashSolver {

int getMaxInputIndex(const Array<int> &inds) {
  CHECK(!inds.empty());
  int m = 0;
  for (auto i: inds) {
    CHECK(0 <= i);
    m = std::max(m, i);
  }
  return m;
}

Player::Player(Spani span) : _strategySpan(span) {}

double Player::eval(const double *X) const {
  double sum = 0.0;
  for (auto f: _functions) {
    sum += f->eval(X);
  }
  return sum;
}

double Player::eval(const double *X,
    double *JtFout,
    std::vector<Eigen::Triplet<double>> *JtJout) {
  double sum = 0.0;
  for (auto f: _functions) {
    sum += f->eval(X, JtFout, JtJout);
  }
  return sum;
}

void Player::add(SubFunction::Ptr p) {
  _functions.push_back(p);
  _jacobianElementCount += p->JtJElementCount();
  _minInputSize = std::max(_minInputSize, 1 + p->maxInputIndex());
}

int Player::minInputSize() const {
  return _minInputSize;
}

int Player::JtJElementCount() const {
  return _jacobianElementCount;
}

Spani Player::strategySpan() const {
  return _strategySpan;
}



}
}
