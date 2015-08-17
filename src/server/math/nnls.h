/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_MATH_NNLS_H_
#define SERVER_MATH_NNLS_H_

#include <server/common/MDArray.h>

namespace sail {

// A class that holds the NNLS solution
class NNLS {
 public:
  enum StatusCode {Success, BadDimensions, IterationCountExceeded};

  StatusCode status() const {
    return _status;
  }

  const bool successful() const {
    return _status == Success;
  }

  const Arrayd &X() const {
    assert(successful());
    return _X;
  }

  static NNLS solve(MDArray2d A, Arrayd B, bool safe = true);
 private:

  NNLS(Arrayd X, StatusCode status) : _X(X), _status(status) {}

  Arrayd _X;
  StatusCode _status;
};


}



#endif /* SERVER_MATH_NNLS_H_ */
