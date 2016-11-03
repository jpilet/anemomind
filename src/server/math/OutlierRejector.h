/*
 * OutlierRejector.h
 *
 *  Created on: 3 Nov 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_OUTLIERREJECTOR_H_
#define SERVER_MATH_OUTLIERREJECTOR_H_

namespace sail {

class OutlierRejector {
public:
  struct Settings {
    double initialWeight = 0.0001;
    double sigma = 1.0;
  };

  OutlierRejector();
  OutlierRejector(const Settings &settings);
  void update(double weight, double residual);
  double computeSquaredWeight() const;
  double computeWeight() const;
private:
  double _alpha, _beta, _sigma;
};

} /* namespace sail */

#endif /* SERVER_MATH_OUTLIERREJECTOR_H_ */
