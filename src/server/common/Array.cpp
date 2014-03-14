#include "Array.h"
#include <cmath>

namespace sail {


int countSliceSize(int count, SliceFun fun) {
  int counter = 0;
  for (int i = 0; i < count; i++) {
    if (fun(i)) {
      counter++;
    }
  }
  return counter;
}

int countSlicedElements(Arrayi arr, SliceFun fun) {
  int counter = 0;
  int count = arr.size();
  for (int i = 0; i < count; i++) {
    if (fun(arr[i])) {
      counter++;
    }
  }
  return counter;
}

Arrayi makeIndexMap(int oldCount, SliceFun fun) {
  Arrayi map(oldCount);
  int counter = 0;
  for (int i = 0; i < oldCount; i++) {
    if (fun(i)) {
      map[i] = counter;
      counter++;
    } else {
      map[i] = -1;
    }
  }
  return map;
}

int indexOf(int index, Arrayi inds) {
  int count = inds.size();
  for (int i = 0; i < count; i++) {
    if (index == inds[i]) {
      return i;
    }
  }
  return -1;
}

Arrayi makeRange(int count, int offset) {
  Arrayi dst(count);
  for (int i = 0; i < count; i++) {
    dst[i] = i + offset;
  }
  return dst;
}

int countTrue(Arrayb array) {
  int count = array.size();
  int counter = 0;
  for (int i = 0; i < count; i++) {
    if (array[i]) {
      counter++;
    }
  }
  return counter;
}

Arrayb ind2log(int len, Arrayi inds) {
  Arrayb log = Arrayb::fill(len, false);
  int count = inds.size();
  for (int i = 0; i < count; i++) {
    log[inds[i]] = true;
  }
  return log;
}
Arrayb neg(Arrayb X) {
  int count = X.size();
  Arrayb Y(count);
  for (int i = 0; i < count; i++) {
    Y[i] = not X[i];
  }
  return Y;
}

bool all(Arrayb X) {
  int count = X.size();
  for (int i = 0; i < count; i++) {
    if (not X[i]) {
      return false;
    }
  }
  return true;
}

bool any(Arrayb X) {
  int count = X.size();
  for (int i = 0; i < count; i++) {
    if (X[i]) {
      return true;
    }
  }
  return false;
}

Arrayi makeSparseInds(int arraySize, int sampleCount) {
  double x0 = 0;
  double x1 = sampleCount-1;
  int y0 = 0;
  int y1 = arraySize-1;
  double k = (y1 - y0)/(x1 - x0);
  Arrayi dst(sampleCount);
  for (int i = 0; i < sampleCount; i++) {
    dst[i] = round(i*k);
  }
  return dst;
}



}



