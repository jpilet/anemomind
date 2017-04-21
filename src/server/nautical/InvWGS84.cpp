/*
 * XYZ2WGS84.cpp
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#include <server/nautical/WGS84.h>
#include <Eigen/Dense>
#include <ceres/ceres.h>
#include "InvWGS84.h"
#include <server/common/StreamUtils.h>

namespace sail {

namespace {
  template <typename T>
  GeographicPosition<T> paramsToGeoPos(const T *params) {
    auto lon = Angle<T>::radians(params[0]);
    auto lat = Angle<T>::radians(params[1]);
    auto alt = Length<T>::meters(params[2]);
    return GeographicPosition<T>(lon, lat, alt);
  }

  class Objf {
  public:
    Objf(const Length<double> *dstXYZ) : _dstXYZ(dstXYZ) {}

    template<typename T>
    bool operator()(T const* const* parameters, T* residuals) const {
      return eval(parameters[0], residuals);
    }
  private:
    template <typename T>
    bool eval(const T *parameters, T *residuals) const {
      Length<T> xyz[3];
      typedef WGS84<T> cvt;
      cvt::toXYZ(paramsToGeoPos(parameters), xyz);
      for (int i = 0; i < 3; i++) {
        residuals[i] = xyz[i].meters() - T(_dstXYZ[i].meters());
      }
      return true;
    }
    const Length<double> *_dstXYZ;
  };

}

GeographicPosition<double> computeGeographicPositionFromXYZ(const Length<double> *xyzL) {
  Eigen::Vector3d xyz(xyzL[0].meters(), xyzL[1].meters(), xyzL[2].meters());
  double len = xyz.norm();
  double lonRadians = atan2(double(xyz(1)), double(xyz(0)));
  double latRadians = asin(double(xyz(2)/len));
  double altitudeMeters = len - (4.0e7/(2.0*M_PI));
  double params[3] = {lonRadians, latRadians, altitudeMeters};
  ceres::Problem problem;
  auto objf = new Objf(xyzL);
  auto cost = new ceres::DynamicAutoDiffCostFunction<Objf>(objf);
  cost->AddParameterBlock(3);
  cost->SetNumResiduals(3);
  problem.AddResidualBlock(cost, nullptr, params);
  ceres::Solver::Summary summary;
  ceres::Solver::Options opts;
  opts.linear_solver_type = ceres::LinearSolverType::DENSE_NORMAL_CHOLESKY;
  opts.minimizer_progress_to_stdout = false;
  opts.logging_type = ceres::SILENT;
  {
    // The reason why we might fail to control the logging
    // is that we use MINIGLOG in Ceres. The reason for
    // using MINIGLOG instead of GLOG was that GLOG was compiled
    // with an ancient version of GCC whose ABI was incompatible
    // with our code. It could be that things have changed.
    //
    // In case the above does not work, this hack might :-)
    RedirectOstream redirectOut(&(std::cout), nullptr);
    RedirectOstream redirectErr(&(std::cerr), nullptr);
    ceres::Solve(opts, &problem, &summary);
  }
  return paramsToGeoPos(params);
}

} /* namespace sail */
