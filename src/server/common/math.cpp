#include "math.h"
#include <assert.h>

namespace sail {

void makeTriBasisVector(int N, int index, double *dst) {
  double lambda0, lambda1;
  solveQuadratic<double>(N, -2, -1, &lambda0, &lambda1);
  assert(std::isfinite(lambda0));
  for (int i = 0; i < N; i++) {
    dst[i] = -lambda0;
  }
  dst[index] += 1.0;
  normalizeInPlace(N, dst);
}

}
