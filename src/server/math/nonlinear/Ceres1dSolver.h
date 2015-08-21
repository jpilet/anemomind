/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_MATH_NONLINEAR_CERES1DSOLVER_H_
#define SERVER_MATH_NONLINEAR_CERES1DSOLVER_H_

#include <server/math/nonlinear/BandedSolver.h>
#include <ceres/ceres.h>

namespace sail {
namespace Ceres1dSolver {

struct Settings {
  BandedSolver::Settings commonSettings;
  enum LossType {L1};

  LossType dataLoss = L1;
  LossType regLoss = L1;
};

ceres::LossFunction *makeLossFunction(Settings::LossType t, double lb) {
  switch (t) {
   case Settings::L1:
     return new ceres::SoftLOneLoss(lb);
  };
  return nullptr;
}


template <int Dim>
class DataCost {
 public:
  DataCost(const Sampling &sampling,
      Array<Observation<1> > observations,
      Settings settings) :
        _sampling(sampling),
        _observations(observations),
        _settings(settings) {}

  template<typename T>
      bool operator()(const T* const x, T* residual) const {
    const MDArray<T, 2> data(_sampling.count(), Dim, x[0]);
    eval(data, residual);
  }

  int outDims() const {
    return _observations.size();
  }

  int inDims() const {
    return Dim*_sampling.count();
  }
 private:
  Sampling _sampling;
  Array<Observation<1> > _observations;
  Settings _settings;


};



template <typename T>
ceres::DynamicAutoDiffCostFunction<T> makeCeresCost(const T *objf) {
  auto cost = new ceres::DynamicAutoDiffCostFunction<T>(objf);
  cost->AddParameterBlock(objf->inDims());
  cost->SetNumResiduals(objf->outDims());
  return cost;
}

template <int Dim>
MDArray2d solve(Sampling sampling,
    Array<Observation<Dim> > observations, Settings settings,
    MDArray2d initialX = MDArray2d()) {
  Arrayd regCoefs = BandMatInternal::makeCoefs(settings.commonSettings.regOrder);
  MDArray2d X = (initialX.empty()?
      BandMatInternal::initialize(sampling.count(), Dim) : initialX);
  const int paramCount = Dim*sampling.count();

  ceres::Problem problem;
  auto dataCost = makeCeresCost(new RegCost<Dim>(sampling, observations, settings));
  problem.AddResidualBlock(regCost,
      makeLossFunction(settings.regLoss,
      settings.commonSettings.residualLowerBound));


}


};
}

#endif /* SERVER_MATH_NONLINEAR_CERES1DSOLVER_H_ */
