/*
 * LsqNashSolver.cpp
 *
 *  Created on: 6 Jan 2017
 *      Author: jonas
 */

#include "LsqNashSolver.h"
#include <server/common/indexed.h>
#include <server/math/EigenUtils.h>
#include <server/common/ArrayIO.h>

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

bool ApproximatePolicy::acceptable(
    const Array<Player::Ptr> &players,
    const State &current,
    const State &candidate) const {
  for (int i = 0; i < players.size(); i++) {
    Eigen::VectorXd tmp = candidate.X;
    auto player = players[i];
    auto span = player->strategySpan();
    tmp.block(span.minv(), 0, span.size(), 1)
        = current.X.block(span.minv(), 0, span.size(), 1);
    if (player->eval(tmp.data())
        < player->eval(candidate.X.data())) {
      return false;
    }
  }
  return true;
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
    if (p.first < players.size()-1) {
      int u = sp.maxv();
      int l = players[p.first+1]->strategySpan().minv();
      if (u != l) {
        LOG(ERROR) << "The strategies are not contiguous and ordered";
        return false;
      }
    }
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

typedef Eigen::SparseMatrix<double, Eigen::ColMajor> SparseMat;
typedef Eigen::Triplet<double> Triplet;

int countJtJElements(const Array<Player::Ptr> &p) {
  int n = 0;
  for (auto x: p) {
    n += x->JtJElementCount();
  }
  return n;
}

struct FullPlayerEval {
  int dim;
  Array<double> values;
  Eigen::VectorXd JtF;
  std::shared_ptr<std::vector<Triplet>> triplets;
  double maxDiag;

  SparseMat makeJtJ(double mu) const;
};

SparseMat FullPlayerEval::makeJtJ(double mu) const {
  int n = triplets->size() + dim;
  std::vector<Triplet> withDiag = *triplets;
  for (int i = 0; i < dim; i++) {
    withDiag.push_back(Triplet(i, i, mu));
  }
  SparseMat m(dim, dim);
  m.setFromTriplets(withDiag.begin(), withDiag.end());
  m.makeCompressed();
  return m;
}

FullPlayerEval eval(
    const Array<Player::Ptr> &players,
    const Eigen::VectorXd &Xinit) {
  int n = Xinit.size();
  Eigen::VectorXd JtF = Eigen::VectorXd::Zero(n);
  auto triplets = std::make_shared<std::vector<Triplet>>();
  triplets->reserve(countJtJElements(players));
  Array<double> values(players.size());
  for (auto p: indexed(players)) {
    values[p.first] = p.second->eval(
        Xinit.data(), JtF.data(), triplets.get());
  }
  double maxDiag = 0.0;
  for (auto e: *triplets) {
    if (e.row() == e.col()) {
      maxDiag = std::max(maxDiag, e.value());
    }
  }
  return FullPlayerEval{n,
    values, JtF, triplets, maxDiag};
}

State evaluateState(
    const Array<Player::Ptr> &players,
    const Eigen::VectorXd &X) {
  Array<double> values(players.size());
  for (auto x: indexed(players)) {
    values[x.first] = x.second->eval(X.data());
  }
  return State{values, X};
}

/*
How to compute rho:

T improvement = currentCost - newCost;
T denom = step.dot(mu*step + minusJtF);
T rho = improvement/denom;

But we cannot do that, because we don't have a single cost
that we are minimizing.

 */
Results solve(
    const Array<Player::Ptr> &players,
    const Eigen::VectorXd &Xinit,
    AcceptancePolicy *policy,
    const Settings &settings) {
  CHECK(validInput(players, Xinit));
  Results results;
  double v = 2.0;
  double mu = -1.0;

  Eigen::VectorXd X = Xinit;
  Eigen::SparseLU<SparseMat> decomp;
  for (int i = 0; i < settings.iters; i++) {
    double currentCost = 0.0;

    auto e = eval(players, X);
    State current{e.values, X};

    if (i == 0) {
      mu = settings.tau*e.maxDiag;
    }

    if (1 <= settings.verbosity) {
      LOG(INFO) << "--------- LevMar Iteration " << i;
      if (2 <= settings.verbosity) {
        LOG(INFO) << " X = " << X.transpose();
        LOG(INFO) << "Descent direction: " << -e.JtF.transpose();
        LOG(INFO) << "Current values: " << current.values;
      }
    }

    bool found = false;
    for (int j = 0; j < settings.subIters; j++) {
      results.X = X;
      if (2 <= settings.verbosity) {
        LOG(INFO) << "   #### Inner iteration " << j;
        LOG(INFO) << "   Damping: " << mu;
      }
      if (!std::isfinite(mu)) {
        LOG(INFO) << "   Damping is no longer finite, cancel optimization.";
        results.type = Results::MuNotFinite;
        return results;
      }
      auto dampedJtJ = e.makeJtJ(mu);
      if (i == 0 && j == 0) {
        decomp.analyzePattern(dampedJtJ);
      }
      decomp.factorize(dampedJtJ);

      Eigen::VectorXd step = decomp.solve(-e.JtF);
      if (3 <= settings.verbosity) {
        LOG(INFO) << "   Proposed step: " << step.transpose();
      }

      Eigen::VectorXd Xnew = X + step;

      State candidate = evaluateState(players, Xnew);

      if (policy->acceptable(players, current, candidate)) {
        if (2 <= settings.verbosity) {
          LOG(INFO) << "   Accept the update";
        }
        policy->accept(players, candidate);
        X = Xnew;
        mu *= 0.5; //acceptedUpdateFactor(rho); Unfortunately, we cannot compute rho.
        v = 2.0;

        found = true;
        break;
      } else {
        if (2 <= settings.verbosity) {
          LOG(INFO) << "   Reject the update";
        }
        mu *= v;
        v *= 2.0;
      }
    }
    if (!found) {
      LOG(ERROR) << "Iterations exceeded";
      results.type = Results::IterationsExceeded;
      return results;
    }
    results.iterationsCompleted = i+1;
  }

  if (1 <= settings.verbosity) {
    LOG(INFO) << "Max iteration count " << settings.iters << " reached";
  }

  results.type = Results::MaxIterationsReached;
  results.X = X;
  return results;
}



}
}
