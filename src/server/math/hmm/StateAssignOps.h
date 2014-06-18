/*
 *  Created on: 2014-06-18
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef STATEASSIGNOPS_H_
#define STATEASSIGNOPS_H_

#include <server/math/hmm/StateAssign.h>

namespace sail {

/*
 * Create a state assign that adds together the state- and transition
 * costs of two other state-assigns. This can be used to implement hinting.
 */
std::shared_ptr<StateAssign> operator+(std::shared_ptr<StateAssign> A,
                                       std::shared_ptr<StateAssign> B);

} /* namespace sail */

#endif /* STATEASSIGNOPS_H_ */
