#ifndef MDINDS_H_
#define MDINDS_H_

namespace sail {

// mirrorIndex: used for clean handling at the boundaries when filtering images.
// E.g. mirrorIndex(-1, 3) -> 0
//      mirrorIndex( 0, 3) -> 0
//      mirrorIndex( 1, 3) -> 1
//      mirrorIndex( 2, 3) -> 2
//      mirrorIndex( 3, 3) -> 2
inline int mirrorIndex(int index, int size) {
  if (index < 0) {
    return mirrorIndex(-index - 1, size);
  } else if (index >= size) {
    return size - 1 - mirrorIndex(index - size, size);
  }
  return index;
}

template <int dims>
class Index {
 public:
  static int calc(const int *inds, const int *sizes) {
    return inds[0] + sizes[0]*Index<dims-1>::calc(inds + 1, sizes + 1);
  }

  static int calcMirrored(const int *inds, const int *sizes) {
    return mirrorIndex(inds[0], sizes[0]) + sizes[0]*Index<dims-1>::calcMirrored(inds + 1, sizes + 1);
  }

  static int numel(const int *sizes) {
    return sizes[0]*Index<dims-1>::numel(sizes+1);
  }

  static void calcInv(int index, const int *sizes, int *indsOut) {
    int s = sizes[0];
    indsOut[0] = index % s;
    Index<dims-1>::calcInv(index/s, sizes + 1, indsOut + 1);
  }

  static bool valid(const int *inds, const int *sizes) {
    return (0 <= inds[0]) && (inds[0] < sizes[0]) && Index<dims-1>::valid(inds + 1, sizes + 1);
  }

  static bool validIncl(const int *inds, const int *sizes) {
    return (0 <= inds[0]) && (inds[0] <= sizes[0]) && Index<dims-1>::validIncl(inds + 1, sizes + 1);
  }

  static bool step(int *inds, const int *sizes, int stepsize) {
    int inds0Added = inds[0] + stepsize;
    int s = sizes[0];
    int inds0New = inds0Added % s;
    int nextStepsize = inds0Added / s;
    inds[0] = inds0New;
    return Index<dims-1>::step(inds + 1, sizes + 1, nextStepsize);
  }

  static bool equals(const int *a, const int *b) {
    return (a[0] == b[0]) && Index<dims-1>::equals(a+1, b+1);
  }
};

template <>
class Index<0> {
 public:
  static bool equals(const int *a, const int *b) {
    return true;
  }

  static int calc(const int *inds, const int *sizes) {
    return 0;
  }

  static int calcMirrored(const int *inds, const int *sizes) {
    return 0;
  }

  static bool valid(const int *inds, const int *sizes) {
    return true;
  }

  static bool validIncl(const int *inds, const int *sizes) {
    return true;
  }

  static bool step(int *inds, const int *sizes, int stepsize) {
    return stepsize != 0;
  }

  static void calcInv(int index, const int *sizes, int *indsOut) {
  }

  static int numel(const int *sizes) {
    return 1;
  }
};

template <int dims>
class MDInds {
 public:
  typedef MDInds<dims> ThisType;

  MDInds() {
    for (int i = 0; i < dims; i++) {
      _sizes[i] = 0;
    }
  }

  MDInds(int *from, int *to) {
    for (int i = 0; i < dims; i++) {
      _sizes[i] = to[i] - from[i];
    }
  }

  MDInds(const int *sizes) {
    for (int i = 0; i < dims; i++) {
      _sizes[i] = sizes[i];
    }
  }

  MDInds(int s0) {
    static_assert(dims == 1, "Bad dim");
    _sizes[0] = s0;
  }

  MDInds(int s0, int s1) {
    static_assert(dims == 2, "Bad dim");
    _sizes[0] = s0;
    _sizes[1] = s1;
  }

  MDInds(int s0, int s1, int s2) {
    static_assert(dims == 3, "Bad dim");
    _sizes[0] = s0;
    _sizes[1] = s1;
    _sizes[2] = s2;
  }

  int calcIndex(const int *inds) const {
    return Index<dims>::calc(inds, _sizes);
  }

  int calcIndexMirrored(const int *inds) const {
    return Index<dims>::calcMirrored(inds, _sizes);
  }

  bool step(int *inds, int step) const {
    return Index<dims>::step(inds, _sizes, step);
  }

  bool valid(int *inds) const {
    return Index<dims>::valid(inds, _sizes);
  }

  bool validIncl(const int *inds) const {
    return Index<dims>::validIncl(inds, _sizes);
  }

  int calcIndex(int index) const {
    static_assert(dims == 1, "The array is not one dimensional");
    return calcIndex(&index);
  }

  int calcIndex(int i, int j) const {
    static_assert(dims == 2, "The array is not two dimensional");
    int inds[2] = {i, j};
    return calcIndex(inds);
  }

  int calcIndex(int i, int j, int k) const {
    static_assert(dims == 3, "The array is not three dimensional");
    int inds[3] = {i, j, k};
    return calcIndex(inds);
  }

  int numel() const {
    return Index<dims>::numel(_sizes);
  }

  void calcInv(int index, int *indsOut) const {
    Index<dims>::calcInv(index, _sizes, indsOut);
  }

  int operator[] (int index) const {
    return _sizes[index];
  }

  void set(int index, int value) {
    _sizes[index] = value;
  }

  int get(int index) const {
    return _sizes[index];
  }

  virtual ~MDInds() {}

  bool operator==(const ThisType &other) const {
    return Index<dims>::equals(_sizes, other._sizes);
  }

  int *getData() {
    return _sizes;
  }

  const int *getData() const {
    return _sizes;
  }
 private:
  int _sizes[dims];
};



} /* namespace sail */
#endif /* MDINDS_H_ */
