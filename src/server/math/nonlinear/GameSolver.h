/*
 * GameSolver.h
 *
 *  Created on: 29 Dec 2016
 *      Author: jonas
 *
 * Implementation of Eq. 36 in
 *
@inproceedings{ratliff_characterization_2013,
  title = {Characterization and computation of local nash equilibria in continuous games},
  url = {http://ieeexplore.ieee.org/xpls/abs_all.jsp?arnumber=6736623},
  urldate = {2016-12-29},
  booktitle = {Communication, {Control}, and {Computing} ({Allerton}), 2013 51st {Annual} {Allerton} {Conference} on},
  publisher = {IEEE},
  author = {Ratliff, Lillian J. and Burden, Samuel A. and Sastry, S. Shankar},
  year = {2013},
  pages = {917--924},
  file = {RatliffCharacterization2013.pdf:/Users/jonas/Library/Application Support/Firefox/Profiles/71ax27z9.default/zotero/storage/EHBGD8G4/RatliffCharacterization2013.pdf:application/pdf}
}

Use Armijo Rule?
https://en.wikipedia.org/wiki/Wolfe_conditions

This seems easier:
http://www.onmyphd.com/?p=gradient.descent#h4_barzilaiborwein

 */

#ifndef SERVER_MATH_NONLINEAR_GAMESOLVER_H_
#define SERVER_MATH_NONLINEAR_GAMESOLVER_H_

#include <server/common/Array.h>
#include <adolc/adouble.h>

namespace sail {
namespace GameSolver {

typedef std::function<adouble(Array<Array<adouble>>)> Function;
typedef std::function<void(int, Array<Array<double>>)> IterationCallback;

struct Settings {
  int iterationCount = 120;
  double stepSize = 0.01;
  IterationCallback iterationCallback;
  short tapeIndex = 0;
};

double evaluatePartialGradient(
    int i, Function f,
    const Array<Array<double>> &X,
    const Settings &settings,
    double *Y);

Array<Array<double>> optimize(
    Array<Function> objectives,
    Array<Array<double>> initialEstimate,
    const Settings &settings);


}
}

#endif /* SERVER_MATH_NONLINEAR_GAMESOLVER_H_ */
