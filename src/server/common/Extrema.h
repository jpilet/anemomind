/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef EXTREMA_H_
#define EXTREMA_H_

namespace sail {

template <typename T, typename ExtremumDetector>
Arrayi genericLocalExtrema(Array<T> arr) {
  int count = arr.size();
  int last = count-1;
  Arrayi extrema(count);
  int counter = 0;
  ExtremumDetector d;
  for (int i = 1; i < last; i++) {
    if (d(arr[i-1], arr[i], arr[i+1])) {
      extrema[counter] = i;
      counter++;
    }
  }
  return extrema;
}

template <typename T>
Arrayi localMaxima(Array<T> arr) {
  class Detect {
   public:
    bool operator()(T a, T b, T c) const {
      return a < b && b > c;
    }
  };
  return genericLocalExtrema<T, Detect>(arr);
}

template <typename T>
Arrayi localMinima(Array<T> arr) {
  class Detect {
   public:
    bool operator()(T a, T b, T c) const {
      return a > b && b < c;
    }
  };
  return genericLocalExtrema<T, Detect>(arr);
}

template <typename T>
Arrayi localExtrema(Array<T> arr) {
  class Detect {
   public:
    bool operator()(T a, T b, T c) const {
      return (a > b && b < c) || (a < b && b > c);
    }
  };
  return genericLocalExtrema<T, Detect>(arr);
}

}



#endif /* EXTREMA_H_ */
