/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/SubdivFractals.h>
#include <random>

namespace sail {
namespace SubdivFractals {


BoundedRule::BoundedRule(MaxSlope slope, double alpha, double beta, int newClass) :
  _slope(slope), _alpha(alpha), _beta(beta), _newClass(newClass) {}


AngleRule::AngleRule(double lambda, int newClass) : _lambda(lambda), _newClass(newClass) {}

Vertex AngleRule::combine(const Vertex &a, const Vertex &b, double w) const {
  double value = _lambda*a.value() + (1.0 - _lambda)*b.value();
  return Vertex(value, _newClass);
}
std::string AngleRule::toString() const {
  std::stringstream ss;
  ss << "Rule::Ptr(new AngleRule(" << _lambda << ", " << _newClass << "))";
  return ss.str();
}


Vertex BoundedRule::combine(const Vertex &a, const Vertex &b, double w) const {
  double value = _slope.fitValue(a.value(), b.value(), _alpha, _beta, 0.5*w);
  return Vertex(value, _newClass);
}

std::string BoundedRule::toString() const {
  std::stringstream ss;
  ss << "Rule::Ptr(new BoundedRule(" << slope() << "," << alpha() << "," << beta() << "," << newClass() << "))";
  return ss.str();
}


namespace {
  std::uniform_int_distribution<int> makeClassDistrib(int classCount) {
    return std::uniform_int_distribution<int>(0, classCount - 1);
  }
}

MDArray<Rule::Ptr, 2> makeRandomBoundedRules(int classCount,
    MaxSlope maxSlope, std::default_random_engine &e) {

  MDArray<Rule::Ptr, 2> rules(classCount, classCount);
  std::uniform_real_distribution<double> alphaBetaDistrib(-1, 1);
  auto indexDistrib = makeClassDistrib(classCount);
  for (int i = 0; i < classCount; i++) {
    for (int j = 0; j < classCount; j++) {
      auto rule = Rule::Ptr(new BoundedRule(maxSlope,
          alphaBetaDistrib(e),
          alphaBetaDistrib(e),
          indexDistrib(e)));
      rules(i, j) = rule;
    }
  }
  return rules;
}

Array<Vertex> makeRandomCtrl(int ctrlCount, int classCount, double maxv,
    std::default_random_engine &e) {
  std::uniform_int_distribution<int> indexDistrib = makeClassDistrib(classCount);
  std::uniform_real_distribution<double> valueDistrib(-maxv, maxv);
  Array<Vertex> dst(ctrlCount);
  for (int i = 0; i < ctrlCount; i++) {
    dst[i] = Vertex(indexDistrib(e), valueDistrib(e));
  }
  return dst;
}

}
}


