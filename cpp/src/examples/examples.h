/*
 * examples.h
 *
 *  Created on: 16 janv. 2014
 *      Author: jonas
 */

#ifndef EXAMPLES_H_
#define EXAMPLES_H_

namespace sail
{

void runTests();

void example001(); // Plotting
void example002(); // Dynamically allocated array
void example003(); // Load all nav data (and plot a time-vs-index graph)
void example004(); // Display time information regarding data.
void example005(); // Plot a single trajectory in 3D
void example006(); // Validate the range object
void example007(); // Compute boundingboxes and check.
void example008(); // Initialize a local race
void example009(); // Pareto frontier test
void example010(); // Grid vertex weight calculation
void example011(); // Make regularization matrices and validate
void example012(); // Noisy signal toy example. Constant regularization.
void example013(); // Noisy signal toy example. Variable regularization.
void example014(); // Noisy signal toy example. Variable regularization. First and Second order.

} /* namespace sail */

#endif /* EXAMPLES_H_ */
