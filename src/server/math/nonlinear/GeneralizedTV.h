/*
 *  Created on: 2014-10-09
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef GENARALIZEDTV_H_
#define GENARALIZEDTV_H_

#include <server/math/UniformSamples.h>
#include <server/math/nonlinear/BandedSolver.h>

namespace sail {

/*
 * Performs generalized total variation (TV) denoising of a signal.
 *
 * For an introduction to the classical first order total variation denoising, see
 *
 * http://en.wikipedia.org/wiki/Total_variation_denoising
 *
 *
 * Here is a
 * relevant paper on generalized TV denoising:
 *
      @article{karahanoglu2011signal,
        title={A signal processing approach to generalized 1-D total variation},
        author={Karahanoglu, Fikret Isik and Bayram, Ilker and Van De Ville, Dimitri},
        journal={Signal Processing, IEEE Transactions on},
        volume={59},
        number={11},
        pages={5265--5274},
        year={2011},
        publisher={IEEE}
}

  To minimize the objective function, the Majorization-Minimization
  algorithm is used:

  @INPROCEEDINGS{4107109,
    author={Figueiredo, M.A.T. and Dias, J.B. and Oliveira, J.P. and Nowak, R.D.},
    booktitle={Image Processing, 2006 IEEE International Conference on},
    title={On Total Variation Denoising: A New Majorization-Minimization Algorithm and an Experimental Comparisonwith Wavalet Denoising},
    year={2006},
    month={Oct},
    pages={2633-2636},
    keywords={image denoising;wavelet transforms;image denoising;majorization-minimization algorithm;total variation method;wavelet-based method;Bayesian methods;Focusing;Image denoising;Image restoration;Noise reduction;Partial differential equations;TV;Telecommunication computing;Wavelet coefficients;Wavelet domain;Image restoration;image denoising;majorization-minimization algorithms;total variation},
    doi={10.1109/ICIP.2006.313050},
    ISSN={1522-4880},}

 *
 */
class GeneralizedTV {
 public:
  GeneralizedTV(int iters = 30, double minv = 1.0e-6,
      double gaussElimTol = 1.0e-6);

  UniformSamplesd filter(UniformSamplesd initialSignal,
                  Arrayd X, Arrayd Y,
                  int order,
                  double regularization) const;

  UniformSamplesd filter(Arrayd X, Arrayd Y, double samplingDensity,
                    int order,
                    double regularization) const;

  // The values of the signal Y are assumed to be
  // have a corresponding X vector ranging from 0 to Y.size()-1.
  // A signal is returned that can be evaluated at those locations.
  UniformSamplesd filter(Arrayd Y, int order, double regularization) const;

  static Arrayd makeDefaultX(int size);
  static UniformSamplesd makeInitialSignal(Arrayd Y);
  static UniformSamplesd makeInitialSignal(Arrayd X, Arrayd Y, double sampleSpacing);
 private:
  BandedSolver::Settings _settings;
};



}

#endif /* GENARALIZEDTV_H_ */
