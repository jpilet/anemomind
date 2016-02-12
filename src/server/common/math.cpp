#include "math.h"
#include <assert.h>

namespace sail {

void makeTriBasisVector(int N, int index, double *dst) {
  assert(0 <= N);
  assert(0 <= index && index < N);
  double lambda0, lambda1;
  solveQuadratic<double>(N, -2, -1, &lambda0, &lambda1);
  assert(std::isfinite(lambda0));
  for (int i = 0; i < N; i++) {
    dst[i] = -lambda0;
  }
  dst[index] += 1.0;
  normalizeInPlace(N, dst);
}

Arrayd makeNextRegCoefs(Arrayd coefs) {
  int n = coefs.size();
  Arrayd next(n+1);
  next[n] = 0.0;
  coefs.copyToSafe(next.sliceTo(n));
  for (int i = 0; i < n; i++) {
    next[i+1] -= coefs[i];
  }
  return next;
}

Arrayd makeRegCoefs(int order) {
  if (order == 0) {
    return Arrayd{1.0};
  } else {
    return makeNextRegCoefs(makeRegCoefs(order - 1));
  }
}

}
