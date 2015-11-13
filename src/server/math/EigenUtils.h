/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef SERVER_MATH_EIGENUTILS_H_
#define SERVER_MATH_EIGENUTILS_H_

#include <Eigen/Core>

namespace sail {
namespace EigenUtils {

struct ABPair {
  Eigen::MatrixXd A;
  Eigen::VectorXd B;
};

ABPair compress(const ABPair &pair);

}
}

#endif /* SERVER_MATH_EIGENUTILS_H_ */
