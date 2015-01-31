#include "Uniform.h"
#include <cstdlib>
#include <ctime>
#include <server/common/LineKM.h>

namespace sail {

void Uniform::randomize() {
  initialize(makeSeed());
}

void Uniform::initialize(time_t seed) {
  srand(seed);
}

time_t Uniform::makeSeed() {
  return time(nullptr);
}




Uniform::Uniform() {
  set(0.0, 1.0);
}

Uniform::Uniform(int count) {
  set(0.0, count);
}

Uniform::Uniform(double a, double b) {
  set(a, b);
}

double Uniform::gen() {
  return _k*rand() + _m;
}

int Uniform::genInt() {
  return int(gen());
}

Uniform::~Uniform() {
}


void Uniform::set(double a, double b) {
  calcLineKM<double>(0.0, RAND_MAX, a, b, _k, _m);
}


} /* namespace sail */
