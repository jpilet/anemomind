/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_MATH_GRIDSEARCH_H_
#define SERVER_MATH_GRIDSEARCH_H_

#include <server/common/Array.h>
#include <server/common/MDArray.h>
#include <limits>
#include <cmath>

namespace sail {
namespace GridSearch {

typedef std::function<double(Arrayi)> Objf;

struct Settings {
 int iters;
 int maxStep;

 Settings() : iters(30), maxStep(30) {}
};


template <int dims>
MDArray<double, dims> makeGrid(int maxStep) {
  auto sizes = Arrayi::fill(dims, 1 + 2*maxStep);
  auto dst = MDArray<double, dims>(sizes.ptr());
  dst.setAll(std::numeric_limits<double>::infinity());
  return dst;
}

template <int dims>
Arrayi initializeGridPos(int maxStep) {
  return Arrayi::fill(dims, maxStep);
}

template <int dims>
Arrayi pos2X(Arrayi Xinit, Arrayi pos, int maxStep) {
  Arrayi dst(dims);
  for (int i = 0; i < dims; i++) {
    dst[i] = Xinit[i] + pos[i] - maxStep;
  }
  return dst;
}


struct Candidate {
  int index;
  double cost;

  bool operator<(const Candidate &other) const {
    return cost < other.cost;
  };
};

template <int dims>
Candidate consider(Objf objf, MDArray<double, dims> *grid, Arrayi *posp, int dim, int step) {
  Arrayi &pos = *posp;
  int originalIndex = pos[dim];
  pos[dim] += step;
  int index = pos[dim];
  double cost = std::numeric_limits<double>::infinity();
  if (grid->valid(pos.ptr())) {
    cost = ((*grid)(pos.ptr()));
    if (std::isinf(cost)) {
      cost = objf(pos);
      (*grid)(pos.ptr()) = cost;
    }
  }
  pos[dim] = originalIndex;
  return Candidate{index, cost};
}

template <int dims>
int optimizeAlongDim(Objf objf, MDArray<double, dims> *grid,
    Arrayi *pos, int dim) {
  return std::min(consider(objf, grid, pos, dim, -1),
          std::min(consider(objf, grid, pos, dim, 0),
                   consider(objf, grid, pos, dim, +1))).index;
}


// The main function to perform optimization
template <int dims>
Arrayi minimize(Objf objf0, Arrayi Xinit, Settings s = Settings()) {
  assert(Xinit.size() == dims);
  auto pos = initializeGridPos<dims>(s.maxStep);
  auto grid = makeGrid<dims>(s.maxStep);
  auto objf = [&](const Arrayi &gridPos) {return objf0(pos2X<dims>(Xinit, gridPos, s.maxStep));};
  for (int i = 0; i < s.iters; i++) {
    Arrayi initPos = pos.dup();
    for (int j = 0; j < dims; j++) {
      pos[j] = optimizeAlongDim(objf, &grid, &pos, j);
    }
    if (initPos == pos) {
      break;
    }
  }
  return pos2X<dims>(Xinit, pos, s.maxStep);
}

}
}

#endif /* SERVER_MATH_GRIDSEARCH_H_ */
