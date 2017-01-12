/*
 * LsqNashSolver.cpp
 *
 *  Created on: 6 Jan 2017
 *      Author: jonas
 */

#include "LsqNashSolver.h"
#include <server/common/indexed.h>

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

bool validInput(
    const Array<Player::Ptr> &players,
    const Eigen::VectorXd &Xinit) {

  if (players.empty()) {
    LOG(ERROR) << "No players provided";
    return false;
  }

  auto mask = Array<int>::fill(Xinit.size(), -1);
  for (auto p: indexed(players)) {
    auto sp = p.second->strategySpan();
    for (auto i: sp) {
      if (i < 0) {
        LOG(ERROR) << "Index below 0 for player " << p.first;
        return false;
      } else if (!(i < mask.size())) {
        LOG(ERROR) << "Strategy index "
            << i << " of player " << p.first
            << " exceeds dimension " <<
            mask.size() << " of optimized vector";
        return false;
      } else if (mask[i] != -1) {
        LOG(ERROR) << "Two different players, "
            << p.first << " and " << mask[i] << ", cannot "
            << " optimize the same index " << i;
      }
      mask[i] = true;
    }
  }

  for (auto x: indexed(mask)) {
    if (x.second == -1) {
      LOG(ERROR) << "The element with index " << x.first
          << " is not optimized by any player.";
      return false;
    }
  }

  return true;
}




}
}
