
#ifndef MDARRAY_H_
#define MDARRAY_H_

#include "Array.h"
#include "MDInds.h"

using namespace std;

namespace sail {

template <typename T, int dims>
class MDArray {
 public:
  typedef MDArray<T, dims> ThisType;

  MDArray() {}
  MDArray(int size) {
    static_assert(dims == 1, "Constructor only works for 1d arrays");
    allocate(&size);
  }

  bool isCol() {
    return cols() == 1;
  }

  bool isRow() {
    return rows() == 1;
  }

  void reserve(int rows, int cols) {
    static_assert(dims == 2, "Only applicable to matrices");
    int sizes[2] = {rows, cols};
    setSize(sizes);
    _data.reserve(rows*cols);
  }

  MDArray(Array<T> arr) {
    int sizes[dims];
    sizes[0] = arr.size();
    for (int i = 1; i < dims; i++) {
      sizes[i] = 1;
    }
    setSize(sizes);
    _data = arr;
  }

  MDArray(int rows, int cols) {

    static_assert(dims == 2, "Constructor only works for 2d arrays");
    int sizes[2] = {rows, cols};
    allocate(sizes);
  }

  static ThisType eye(int r, int c) {
    static_assert(dims == 2, "Eye matrix only for 2d arrays");
    ThisType result(r, c);
    result.setAll(0.0);
    int n = (r < c? r : c);
    for (int i = 0; i < n; i++) {
      result(i, i) = 1.0;
    }
    return result;
  }

  bool isMatrix(int r, int c) {
    static_assert(dims == 2, "Only for 2d arrays");
    return rows() == r && cols() == c;
  }

  MDArray(int rows, int cols, T *data) {
    static_assert(dims == 2, "Constructor only works for 2d arrays");
    int sizes[2] = {rows, cols};
    setSize(sizes);
    _data = Array<T>(rows*cols, data);
  }

  MDArray(int rows, int cols, Array<T> data) {

    static_assert(dims == 2, "Constructor only works for 2d arrays");
    int sizes[2] = {rows, cols};
    setSize(sizes);
    _data = data;
  }

  MDArray(int rows, int cols, Array<T> data, int step) {

    static_assert(dims == 2, "Constructor only works for 2d arrays");
    int sizes[2] = {rows, cols};
    setSize(sizes);
    _index.set(0, step);
    _data = data;
  }

  void create(int r, int c) {
    static_assert(dims == 2, "Only applicable to matrices.");
    if (rows() != r  || cols() != c) {
      int sizes[2] = {r, c};
      allocate(sizes);
    }
  }

  bool isSquare() const {
    static_assert(dims == 2, "Only applicable to matrices.");
    return rows() == cols();
  }

  void create(const int *sizes) {
    allocate(sizes);
  }

  MDArray(const int *sizes) {
    allocate(sizes);
  }

  MDArray(int *sizes, Array<T> data) {
    setSize(sizes);
    _data = data;
  }

  MDArray(const MDInds<dims> &size, Array<T> data) :
    _data(data), _size(size), _index(size) {}

  void allocate(const int *sizes) {
    setSize(sizes);
    _data = Array<T>(_size.numel());
  }

  ThisType slice(int *from, int *to) const {
#if SAFEARRAY
    assert(_size.validIncl(from));
    assert(_size.validIncl(to));
#endif

    int offset = _index.calcIndex(from);
    return ThisType(_data.sliceFrom(offset), _index, MDInds<dims>(from, to));
  }


  void set1d(int index, const T &value) {
    int inds[dims];
    _size.calcInv(index, inds);
    set(inds, value);
  }

  T get1d(int index) {
    int inds[dims];
    _size.calcInv(index, inds);
    return get(inds);
  }

  void set(int *inds, const T &value) {
#if SAFEARRAY
    assert(_size.valid(inds));
#endif
    _data.set(_index.calcIndex(inds), value);
  }

  void set(std::function<T(int i, int j)> fun) {
    static_assert(dims == 2, "Only applicable to 2d arrays");
    int r = _size.get(0);
    int c = _size.get(1);
    for (int i = 0; i < r; i++) {
      for (int j = 0; j < c; j++) {
        set(i, j, fun(i, j));
      }

    }
  }

  bool valid(int *inds) const {
    return _size.valid(inds);
  }

  bool valid(int i, int j) const {
    static_assert(dims == 2, "Only 2d arrays");
    int inds[2] = {i, j};
    return valid(inds);
  }

  void setAll(const T &value) {
    if (empty()) {
      return;
    }
    int inds[dims];
    initInds(inds);
    while (true) {
      set(inds, value);
      if (step(inds, 1)) {
        break;
      }
    }
  }

  void initInds(int *inds) const {
    for (int i = 0; i < dims; i++) {
      inds[i] = 0;
    }
  }

  void copyTo(ThisType &dst) const {
    dst.create(_size.getData());
    copyToSafe(dst);
  }

  void copyToSafe(ThisType dst) const {
    assert(_size == dst._size);
    int inds[dims];
    initInds(inds);
    while (true) {
      dst.set(inds, get(inds));
      if (step(inds, 1)) {
        break;
      }
    }
  }

  void addToSafe(ThisType dst, T factor = 1) {
    assert(_size == dst._size);
    int inds[dims];
    initInds(inds);
    while (true) {
      dst.set(inds, dst.get(inds) + factor*get(inds));
      if (step(inds, 1)) {
        break;
      }
    }
  }

  ThisType allocate() const {
    ThisType dst(_size.getData());
    return dst;
  }

  ThisType dup() const {
    ThisType dst = allocate();
    copyTo(dst);
    return dst;
  }


  T get(int *inds) const {
#if SAFEARRAY
    assert(_size.valid(inds));
#endif
    return _data.get(_index.calcIndex(inds));
  }

  T *getPtrAt(int *inds) const {
#if SAFEARRAY
    assert(_size.valid(inds));
#endif
    return _data.getData() + _index.calcIndex(inds);
  }

  T getStepAlongDim(int dim) const {
    int inds[dims];
    for (int i = 0; i < dims; i++) {
      inds[i] = 0;
    }
    int offset = _index.calcIndex(inds);
    inds[dim] = 1;
    return _index.calcIndex(inds) - offset;
  }

  T *getPtrAt(int i, int j) const {
    static_assert(dims == 2, "Only applicable to 2d arrays");
    int inds[2] = {i, j};
    return getPtrAt(inds);
  }

  T &operator() (int *inds) {
#if SAFEARRAY
    assert(_size.valid(inds));
#endif
    return _data[_index.calcIndex(inds)];
  }

  const T &operator() (int *inds) const {
#if SAFEARRAY
    assert(_size.valid(inds));
#endif
    return _data[_index.calcIndex(inds)];
  }

  int calcIndex(int *inds) const {
    return _index.calcIndex(inds);
  }

  // One-dimensional read/write
  T &operator[] (int index) {
    return _data[calcInternalIndex(index)];
  }

  void set(int i, int j, const T &value) {
    static_assert(dims == 2, "Array should be 2d");
    int inds[2] = {i, j};
    set(inds, value);
  }

  T get(int i, int j) const {
    static_assert(dims == 2, "Array should be 2d");
    int inds[2] = {i, j};
    return get(inds);
  }

  T &operator() (int i, int j) {
    static_assert(dims == 2, "Array should be 2d");
    int inds[2] = {i, j};
    return operator()(inds);
  }

  const T &operator() (int i, int j) const {
    static_assert(dims == 2, "Array should be 2d");
    int inds[2] = {i, j};
    return operator()(inds);
  }

  void set(int i, const T &value) {
    static_assert(dims == 1, "Array should be 1d");
    set(&i, value);
  }

  T get(int i) {
    static_assert(dims == 1, "Array should be 1d");
    return get(&i);
  }

  T &operator() (int i) {
    static_assert(dims == 1, "Array should be 1d");
    return operator()(&i);
  }


  bool empty() const {
    return numel() == 0;
  }
  int numel() const {
    return _size.numel();
  }
  virtual ~MDArray() {}

  // Returns true upon reaching the end
  bool step(int *inds, int stepsize) const {
    return _size.step(inds, stepsize);
  }

  ThisType sliceAlongDim(int dim, int from, int to) const {
    int fromArr[dims];
    int toArr[dims];
    for (int i = 0; i < dims; i++) {
      fromArr[i] = 0;
      toArr[i] = _size[i];
    }
    fromArr[dim] = from;
    toArr[dim] = to;
    return slice(fromArr, toArr);
  }

  ThisType sliceRows(Arrayi inds) {
    static_assert(dims == 2, "Only 2d arrays");
    int colcount = cols();
    int rowcount = inds.size();
    ThisType dst(rowcount, colcount);
    for (int i = 0; i < inds.size(); i++) {
      for (int j = 0; j < colcount; j++) {
        dst(i, j) = get(inds[i], j);
      }
    }
    return dst;
  }

  ThisType sliceRows(Arrayb sel) {
    static_assert(dims == 2, "Only 2d arrays");
    assert(rows() == sel.size());
    int colcount = cols();
    int rowcount = countTrue(sel);
    ThisType dst(rowcount, colcount);
    int counter = 0;
    for (int i = 0; i < rows(); i++) {
      if (sel[i]) {
        for (int j = 0; j < colcount; j++) {
          dst(counter, j) = get(i, j);
        }
        counter++;
      }
    }
    assert(counter == rowcount);
    return dst;
  }

  ThisType sliceCols(Arrayb sel) {
    static_assert(dims == 2, "Only 2d arrays");
    assert(cols() == sel.size());
    int colcount = countTrue(sel);
    int rowcount = rows();
    ThisType dst(rowcount, colcount);
    int counter = 0;
    for (int j = 0; j < cols(); j++) {
      if (sel[j]) {
        for (int i = 0; i < rowcount; i++) {
          dst(i, counter) = get(i, j);
        }
        counter++;
      }
    }
    assert(counter == colcount);
    return dst;
  }

  ThisType sliceBlock(int dim, int index, int blockSize) const {
    int from = index*blockSize;
    int to = from + blockSize;
    return sliceAlongDim(dim, from, to);
  }

  ThisType sliceRows(int from, int to) const {
    return sliceAlongDim(0, from, to);
  }

  ThisType sliceCols(int from, int to) const {
    static_assert(dims >= 2, "Dimension should be at least 2");
    return sliceAlongDim(1, from, to);
  }

  ThisType sliceColsFrom(int from) const {
    return sliceCols(from, cols());
  }

  ThisType sliceColsTo(int to) const {
    return sliceCols(0, to);
  }

  ThisType sliceRowsFrom(int from) const {
    return sliceRows(from, rows());
  }

  ThisType sliceRowsTo(int to) const {
    return sliceRows(0, to);
  }

  ThisType sliceCol(int index) const {
    return sliceCols(index, index + 1);
  }

  ThisType sliceRow(int index) const {
    return sliceRows(index, index + 1);
  }

  ThisType sliceRowBlock(int index, int blockSize) const {
    return sliceBlock(0, index, blockSize);
  }

  ThisType sliceColBlock(int index, int blockSize) const {
    static_assert(dims >= 2, "Dimension should be at least 2");
    return sliceBlock(1, index, blockSize);
  }

  int getRows() const {
    return _size.get(0);
  }

  int rows() const {
    return getRows();
  }

  int getCols() const {
    static_assert(dims >= 2, "Only applicable to arrays with at least dimension 2");
    return _size.get(1);
  }

  int cols() const {
    return getCols();
  }

  typedef T ElemType;

  T *getData() const {
    return _data.getData();
  }
  T *ptr() const {
    return _data.ptr();
  }

  Array<T> getStorage() {
    return _data;
  }

  bool isContinuous() const {
    // Skip the last dimension since it is not used
    return Index<dims-1>::equals(_index.getData(), _size.getData());
  }

  int getSize(int dim) const {
#if SAFEARRAY
    assert(0 <= dim);
    assert(dim <= dims);
#endif
    return _size.get(dim);
  }


  template <typename Function>
  auto map(const Function &f) const ->
    MDArray<decltype(std::declval<Function>()(std::declval<T>())),
      dims> const {
    typedef decltype(std::declval<Function>()(std::declval<T>())) S;
    MDArray<S, dims> dst(_size.getData());
    int count = numel();
    if (isContinuous()) {
      const T *srcData = ptr();
      S *dstData = dst.ptr();
      for (int i = 0; i < count; i++) {
        dstData[i] = f(srcData[i]);
      }
    } else {
      int inds[dims];
      initInds(inds);
      for (int i = 0; i < count; i++) {
        dst.set(inds, f(get(inds)));
        _size.step(inds, 1);
      }
    }
    return dst;
  }

  template <typename S>
  MDArray<S, dims> cast() const{
    return map([](const T &x) {return static_cast<S>(x);});
  }

  int getStep() const {
    static_assert(dims == 2, "Only applicable to 2d arrays");
    return _index[0];
  }

  int getByteStep() const {
    return sizeof(T)*getStep();
  }

  void setInternalSize(int index, int s) {
    _index.set(index, s);
  }

  int getInternalSize(int index) const {
    return _index[index];
  }

  bool sameSizeAs(MDArray<T, dims> &other) {
    return Index<dims>::equals(_size.getData(), other._size.getData());
  }

  const MDInds<dims> &size() const {
    return _size;
  }

  Array<T> continuousData() const {
    if (isContinuous()) {
      return _data;
    } else {
      int n = numel();
      Array<T> dst(n);
      int inds[dims];
      for (int i = 0; i < n; i++) {
        assert(false); // TODO
      }
      return dst;
    }
  }
 private:
  int calcInternalIndex(int index) const {
    #if SAFEARRAY
        assert(0 <= index);
        assert(index < numel());
    #endif
    int inds[dims];
    _size.calcInv(index, inds);
    return _index.calcIndex(inds);
  }

  void setSize(const int *sizes) {
    _index = MDInds<dims>(sizes);
    _size = MDInds<dims>(sizes);
  }

  MDArray(Array<T> data, MDInds<dims> index, MDInds<dims> size) {
    _data = data;
    _index = index;
    _size = size;
  }

  Array<T> _data;
  MDInds<dims> _index; // Used to compute the index of the array
  MDInds<dims> _size;  // Used to check the size of the array
};

template <typename T>
Array<T> getColumn(MDArray<T, 2> array, int index) {
  int inds[2] = {0, index};
  int from = array.calcIndex(inds);
  int to = from + array.rows();
  return array.getStorage().slice(from, to);
}

typedef MDArray<double, 2> MDArray2d;
typedef MDArray<int, 2> MDArray2i;
typedef MDArray<bool, 2> MDArray2b;
typedef MDArray<unsigned char, 2> MDArray2ub;

template <typename T>
MDArray<T, 2> hcat(MDArray<T, 2> A, MDArray<T, 2> B) {
  assert(A.rows() == B.rows());
  MDArray<T, 2> C(A.rows(), A.cols() + B.cols());
  A.copyToSafe(C.sliceColsTo(A.cols()));
  B.copyToSafe(C.sliceColsFrom(A.cols()));
  return C;
}

template <typename T>
MDArray<T, 2> vcat(MDArray<T, 2> A, MDArray<T, 2> B) {
  assert(A.cols() == B.cols());
  MDArray<T, 2> C(A.rows() + B.rows(), A.cols());
  A.copyToSafe(C.sliceRowsTo(A.rows()));
  B.copyToSafe(C.sliceRowsFrom(A.rows()));
  return C;
}

} /* namespace sail */
#endif /* MDARRAY_H_ */
