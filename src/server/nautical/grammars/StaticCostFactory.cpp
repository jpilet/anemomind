/*
 *  Created on: 2014-06-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "StaticCostFactory.h"
#include <assert.h>
#include <server/common/Hierarchy.h>
#include <server/common/string.h>
#include <iostream>
#include <server/common/Functional.h>

namespace sail {

namespace {
  int listTerminalsPerNode(int currentNode, const Hierarchy &h,
    int counter, Array<Arrayi> dst, Arrayi allTerminals) {
    int next = counter;
    if (h.isTerminal(currentNode)) {
      allTerminals[counter] = currentNode;
      next++;
    } else {
      for (auto c : h.children(currentNode)) {
        next = listTerminalsPerNode(c, h, next, dst, allTerminals);
      }
    }
    dst[currentNode] = allTerminals.slice(counter, next);
    return next;
  }
}

StaticCostFactory::StaticCostFactory(const Hierarchy &h) {
  int terminalCount = -1;
  {
    int n = h.nodeCount();
    Arrayi allTerminals = Arrayi::fill(n, -1);
    _terminalsPerNode = Array<Arrayi>(n);
    terminalCount = listTerminalsPerNode(h.rootNode(), h, 0, _terminalsPerNode, allTerminals);
    assert(all(sail::map(_terminalsPerNode, [&](const Arrayi &x) {
      return x.hasData();
    })));
  }
  int n = terminalCount;
  _stateCosts = Arrayd::fill(n, 0.0);

  _transitionCosts = MDArray2d(n, n);
  _transitionCosts.setAll(0);

  _con = MDArray2b(n, n);
  _con.setAll(false);
}

void StaticCostFactory::connect(int i, int j, std::function<double(int,int)> f,
    bool symmetric) {
  if (symmetric) {
    connect(j, i, f, false);
  }
  Arrayi I = _terminalsPerNode[i];
  Arrayi J = _terminalsPerNode[j];
  for (auto src : I) {
    for (auto dst : J) {
      _con(src, dst) = true;
      double cost = f(src, dst);
      _transitionCosts(src, dst) += cost;
    }
  }
}

void StaticCostFactory::connect(int i, int j, double cost,
    bool symmetric) {
  connect(i, j, [=](int src, int dst) {return cost;}, symmetric);
}

void StaticCostFactory::addStateCost(int i, std::function<double(int)> f) {
  Arrayi I = _terminalsPerNode[i];
  for (auto dst : I) {
    _stateCosts[dst] += f(dst);
  }
}

void StaticCostFactory::addStateCost(int i, double cost) {
  addStateCost(i, [=](int k) {return cost;});
}


} /* namespace sail */
