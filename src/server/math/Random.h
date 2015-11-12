/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef SERVER_MATH_RANDOM_H_
#define SERVER_MATH_RANDOM_H_

#include <random>

namespace sail {

// Would there be any reason
// to use more than one type of RNGs in our project?
// I guess we can hardcode the choice of RNG here.
typedef std::mt19937 RandomEngine;

RandomEngine makeRngForTests();


}

#endif /* SERVER_MATH_RANDOM_H_ */
