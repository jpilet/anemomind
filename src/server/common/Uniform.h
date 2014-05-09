#ifndef UNIFORM_H_
#define UNIFORM_H_

#include <ctime> // to get time_t type.

namespace sail {

class Uniform {
 public:
  static void randomize();
  static time_t makeSeed();
  static void initialize(time_t seed);
  Uniform();
  Uniform(int count); // e.g. 2 => gen() -> [0, 2]         genInt() -> {0, 1}
  Uniform(double a, double b);
  double gen();
  int genInt();
  virtual ~Uniform();
 private:
  void set(double a, double b);
  double _k, _m;
};

} /* namespace sail */
#endif /* UNIFORM_H_ */
