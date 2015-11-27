/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef SERVER_MATH_CERESUTILS_H_
#define SERVER_MATH_CERESUTILS_H_

namespace sail {
namespace CeresUtils {


/*
 * An objective function that only uses the first parameter block.
 *
 * That objective function should have a method with the signature
 *
 * template <typaname T>
 * bool eval(const T *parameters, T *residuals);
 */
template <typename Objf>
class SimpleObjf {
 public:
  SimpleObjf(const std::shared_ptr<Objf> &objf) : _objf(objf) {}

  template<typename T>
  bool operator()(T const* const* parameters, T* residuals) {
    return _objf->eval(parameters[0], residuals);
  }

  virtual ~SimpleObjf() {
    delete _objf;
  }
 private:
  std::shared_ptr<Objf> *_objf;
};

/*
 * For the common case of a single cost
 * function and a single parameter block.
 *
 *
 */
template <typename Objf>
void configureSingleObjfAndParameterBlock(
    ceres::Problem *problem,
    std::shared_ptr<Objf> objf,
    double *parameters) {
  auto cost = new ceres::DynamicAutoDiffCostFunction<SimpleObjf<Objf > >(
      new SimpleObjf<Objf>(objf));
  cost->AddParameterBlock(objf->inDims());
  cost->SetNumResiduals(objf->outDims());
  problem->AddResidualBlock(cost, nullptr, objf->outDims());
}


}
}



#endif /* SERVER_MATH_CERESUTILS_H_ */
