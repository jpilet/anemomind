#ifndef ARRAY_H_
#define ARRAY_H_

#include <functional>
#include <server/common/ArrayStorage.h>

namespace sail {

#ifndef SAFEARRAY
#define SAFEARRAY 1
#endif

typedef std::function<bool(int)> SliceFun;


// Implements a reference counted ("garbage collected") and sliceable generic array type.
template <typename T>
class Array {
 public:
  typedef Array<T> ThisType;
  typedef T ElemType;
  typedef T value_type;

  Array() {
    _size = 0;
    _data = nullptr;
  }

  Array(std::initializer_list<T> args) {
    initialize(args.size());
    int i = 0;
    for (auto arg: args) {
      _data[i++] = arg;
    }
  }

  typedef std::function<T(int index)> ArrayFun;
  static ThisType fill(int size, ArrayFun fun) {
    ThisType dst(size);
    for (int i = 0; i < size; i++) {
      dst.set(i, fun(i));
    }
    return dst;
  }

  // Only if T is an array
  T join() {
    int elemCount = 0;
    for (int i = 0; i < _size; i++) {
      elemCount += _data[i].size();
    }
    T result(elemCount);
    int counter = 0;
    for (int i = 0; i < _size; i++) {
      int next = counter + _data[i].size();
      _data[i].copyToSafe(result.slice(counter, next));
      counter = next;
    }
    assert(counter == elemCount);
    return result;
  }



  static ThisType fill(int size, T value) {
    ThisType dst(size);
    for (int i = 0; i < size; i++) {
      dst.set(i, value);
    }
    return dst;
  }

  template <typename S>
  static ThisType referToVector(S &x) {
    return ThisType(x.size(), x.data());
  }

  bool empty() const {
    return _size == 0;
  }

  bool hasData() const {
    return not empty();
  }

  // Create a new array of specified size
  Array(int size) {
    initialize(size);
  }

  Array(int size, const T *srcData) {
    initialize(size);
    assert(srcData != nullptr);
    for (int i = 0; i < size; i++) {
      _data[i] = srcData[i];
    }
  }

  Array(ArrayStorage<T> storage) {
    _storage = storage;
    if (storage.allocated()) {
      _data = storage.ptr();
      _size = storage.size();
    } else {
      _data = nullptr;
      _size = 0;
    }
  }

  Array(T *data, int size, ArrayStorage<T> storage) {
    _data = data;
    _size = size;
    _storage = storage;
  }

  void copyToSafe(Array<T> dst) {
    assert(dst.size() == _size);
    for (int i = 0; i < _size; i++) {
      dst[i] = _data[i];
    }
  }

  void create(int size) {
    if (_size != size) {
      initialize(size);
    }
  }

  void clear() {
    create(0);
  }

  void reserve(int s) {
    if (size() < s) {
      create(s);
    }
  }

  // Different from reserve in the sense that
  //  * it doubles the storage until it is at least s
  //  * it retains old data.
  // This method is useful to manage a buffer that may grow
  void expandIfNeeded(int s) {
    if (_size < s) {
      int newSize = 2*(_size == 0? 1 : _size);
      while (newSize < s) {
        newSize *= 2;
      }
      T *newData = new T[newSize];
      for (int i = 0; i < _size; i++) {
        newData[i] = _data[i];
      }
      assignOwnage(newSize, newData);
    }
  }

  void expandIfNeededAndFill(int s, const T &fillvalue) {
    if (_size < s) {
      int oldSize = _size;
      expandIfNeeded(s);
      for (int i = oldSize; i < _size; i++) {
        _data[i] = fillvalue;
      }
    }
  }

  static ThisType makeCopy(int count, T *data) {
    ThisType arr(count);
    for (int i = 0; i < count; i++) {
      arr.set(i, data[i]);
    }
    return arr;
  }

  template <typename S>
  static ThisType makeArrayCopy(S &X) {
    int count = X.size();
    ThisType arr(count);
    for (int i = 0; i < count; i++) {
      arr.set(i, X[i]);
    }
    return arr;
  }

  template <typename S>
  Array<S> castNoCopy() const {
    int bytes = sizeof(T)*_size;
    int Scount = bytes/sizeof(S);
    assert(Scount*sizeof(S) == bytes);
    return Array<S>(Scount, (S*)_data);
  }

  template <typename S>
  Array<S> convertToArray() const {
    int bytes = sizeof(T)*_size;
    int Scount = bytes/sizeof(S);
    assert(Scount*sizeof(S) == bytes);
    return Array<S>::makeCopy(Scount, (S*)_data);
  }

  template <typename S>
  Array<S> cast() const {
    return convertToArray<S>();
  }

  Array<double> castd() {
    return cast<double>();
  }

  Array<int> casti() {
    return cast<int>();
  }



  template <typename S>
  S convertTo() {
    int bytes = sizeof(T)*_size;
    assert(bytes == sizeof(S));
    S dst = *((S *)(_data));
    return dst;
  }

  ThisType allocate() {
    return ThisType(_size);
  }

  ThisType dup() const {
    int count = size();
    ThisType dst(count);
    for (int i = 0; i < count; i++) {
      dst.set(i, get(i));
    }
    return dst;
  }

  template <typename S>
  bool same(std::function<S(T)> fun) const {
    assert(hasData());
    S ref = fun(_data[0]);
    for (int i = 1; i < _size; i++) {
      if (!(ref == fun(_data[i]))) {
        return false;
      }
    }
    return true;
  }

  bool all(std::function<bool(T)> fun) const {
    for (int i = 0; i < _size; i++) {
      if (!fun(_data[i])) {
        return false;
      }
    }
    return true;
  }

  static Array<T> own(int size, T *data) {
#if SAFEARRAY
    assert(0 <= size);
#endif
    int *refs = new int;
    *refs = 1;
    Array<T> result(size, data, refs, nullptr);
    return result;
  }


  // Initialize an array to point at existing data
  Array(int size, T *data) {
#if SAFEARRAY
    assert(data != nullptr || size == 0);
    assert(0 <= size);
#endif
    _size = size;
    _data = data;
  }

  void setTo(const T &value) {
    int s = size();
    for (int i = 0; i < s; i++) {
      set(i, value);
    }
  }

  void setTo(Array<T> src) {
#if SAFEARRAY
    assert(_size == src.size());
#endif
    for (int i = 0; i < _size; i++) {
      _data[i] = src.get(i);
    }

  }

  // Get a slice of size to - from
  ThisType slice(int from, int to) const {
#if SAFEARRAY
    assert(from <= to);
    assert(0 <= from);
    assert(to <= _size);
#endif
    if (empty()) {
      assert(_data == nullptr);
      return ThisType();
    }
    return Array(_data + from, to - from, _storage);
  }

  ThisType sliceBlock(int index, int blockSize) {
    int offset = index*blockSize;
    return slice(offset, offset + blockSize);
  }

  ThisType slice(Array<int> subinds) {
    int count = subinds.size();
    ThisType dst(count);
    for (int i = 0; i < count; i++) {
      dst.set(i, get(subinds.get(i)));
    }
    return dst;
  }

  ThisType slice(Array<bool> incl) {
    assert(incl.size() == _size);
    ThisType Y(_size);
    int counter = 0;
    for (int i = 0; i < _size; i++) {
      if (incl[i]) {
        Y[counter] = _data[i];
        counter++;
      }
    }
    return Y.sliceTo(counter);
  }

  ThisType sliceBlocks(int blockSize, Array<bool> incl) {
    assert(blockSize*incl.size() == _size);
    int count = incl.size();
    ThisType Y(_size);
    int counter = 0;
    for (int i = 0; i < count; i++) {
      if (incl[i]) {
        int dstOffset = blockSize*counter;
        int srcOffset = i*blockSize;
        for (int j = 0; j < blockSize; j++) {
          Y[dstOffset + j] = _data[srcOffset + j];
        }
        counter++;
      }
    }
    return Y.sliceTo(counter*blockSize);
  }

  ThisType sliceBlocks(int blockSize, Array<int> inds) {
    int count = inds.size();
    int dstSize = blockSize*count;
    Array<T> dst(dstSize);
    for (int i = 0; i < count; i++) {
      int srcOffset = blockSize*inds[i];
      int dstOffset = blockSize*i;
      for (int j = 0; j < blockSize; j++) {
        dst[dstOffset + j] = _data[srcOffset + j];
      }
    }
    return dst;
  }

  ThisType sliceFrom(int from) const {
    return slice(from, _size);
  }

  ThisType sliceTo(int to) const {
    return slice(0, to);
  }

  ThisType sliceLast(int n) const {
    return sliceFrom(_size - n);
  }

  ThisType sliceBut(int n) const {
    return sliceTo(_size - n);
  }

  // Read/write access to array elements
  inline T &operator[] (int index) {
#if SAFEARRAY
    assert(0 <= index);
    assert(index < _size);
    assert(_data != nullptr);
#endif
    return _data[index];
  }

  inline const T &operator[] (int index) const {
#if SAFEARRAY
    assert(0 <= index);
    assert(index < _size);
    assert(_data != nullptr);
#endif
    return _data[index];
  }

  inline T &rawAccess(int index) {
    return _data[index];
  }

  const T &getRef(int index) const {
#if SAFEARRAY
    assert(0 <= index);
    assert(index < _size);
    assert(_data != nullptr);
#endif
    return _data[index];
  }

  // Read access to array elements
  T get(int index) const {
#if SAFEARRAY
    assert(0 <= index);
    assert(index < _size);
    assert(_data != nullptr);
#endif
    return _data[index];
  }

  // Set elements
  void set(int index, const T &value) {
#if SAFEARRAY
    assert(0 <= index);
    assert(index < _size);
    assert(_data != nullptr);
#endif
    _data[index] = value;
  }

  int size() const {
    return _size;
  }
  int middle() const {
    return _size/2;
  }

  int lastIndex() const {
    return _size - 1;
  }


  bool operator==(const ThisType &other) const {
    if (other.size() != _size) {
      return false;
    }
    for (int i = 0; i < _size; i++) {
      if (!(_data[i] == other._data[i])) {
        return false;
      }
    }

    return true;
  }

  bool byteEquals(const ThisType &other) {
    return cast<unsigned char>() == other.cast<unsigned char>();
  }


  T *getData() const {
    return _data;
  }

  T *ptr() const {
    return _data;
  }

  T *blockPtr(int index, int blockSize) const {
    return ptr() + index*blockSize;
  }

  T *ptr(int index) const {
    return _data + index;
  }

  T *lastPtr() const {
    return _data + _size;
  }

  ThisType append(const T &x) {
    int count = size();
    ThisType dst(count + 1);
    for (int i = 0; i < count; i++) {
      dst.set(i, get(i));
    }
    dst.set(count, x);
    return dst;
  }

  ThisType append(ThisType X) {
    int count0 = size();
    int count1 = X.size();
    int count2 = count0 + count1;
    ThisType dst(count2);
    for (int i = 0; i < count0; i++) {
      dst.set(i, get(i));
    }
    for (int i = count0; i < count2; i++) {
      dst.set(i, X.get(i - count0));
    }
    return dst;
  }

  ThisType slice(std::function<bool(T)> fun) const {
    ThisType dst(_size);
    int counter = 0;
    for (int i = 0; i < _size; i++) {
      if (fun(_data[i])) {
        dst[counter] = _data[i];
        counter++;
      }
    }
    return dst.sliceTo(counter);
  }


  T &last() {
    return _data[_size - 1];
  }

  const T &last() const {
    return _data[_size - 1];
  }

  T &first() {
    return _data[0];
  }

  const T &first() const {
    return _data[0];
  }

  bool identicTo(const Array<T> &other) {
    return _size == other._size  && _data == other._data && _storage == other._storage;
  }

  ThisType cat(ThisType other) {
    ThisType dst(_size + other.size());
    copyToSafe(dst.sliceTo(_size));
    other.copyToSafe(dst.sliceFrom(_size));
    return dst;
  }

  T *output(T *out) {
    for (int i = 0; i < _size; i++) {
      out[i] = _data[i];
    }
    return out + _size;
  }

  ThisType output(ThisType dst) {
    for (int i = 0; i < _size; i++) {
      dst[i] = _data[i];
    }
    return dst.sliceFrom(_size);
  }

  ThisType kron(int dims) {
    int dstsize = dims*_size;
    ThisType dst(dstsize);
    for (int i = 0; i < _size; i++) {
      T &x = _data[i];
      int offs = i*dims;
      for (int j = 0; j < dims; j++) {
        dst[offs + j] = x;
      }
    }
    return dst;
  }

  void decreaseSize(int newSize) {
    assert(newSize <= _size);
    _size = newSize;
  }

  int find(const T &x) {
    for (int i = 0; i < _size; i++) {
      if (_data[i] == x) {
        return i;
      }
    }
    return -1;
  }




  typedef const T *ConstIterator;
  typedef T *Iterator;

  Iterator begin() {
    return Iterator(_data);
  }

  Iterator end() {
    return Iterator(_data + _size);
  }

  ConstIterator begin() const {
    return ConstIterator(_data);
  }

  ConstIterator end() const {
    return ConstIterator(_data + _size);
  }
 private:

  void initialize(int size) {
  #if SAFEARRAY
    assert(0 <= size);
  #endif
    _size = size;
    _storage = ArrayStorage<T>(size);
    _data = _storage.ptr();
  }

  // A pointer to the start of the data
  T*_data;

  // The size of the array
  int _size;

  // Wraps an array of dynamically allocated memory. Can be empty in case _data points to the stack.
  ArrayStorage<T> _storage;
};

typedef Array<double> Arrayd;
typedef Array<bool> Arrayb;
typedef Array<int> Arrayi;
typedef Array<unsigned char> Arrayub;

template <typename T>
Array<T> addScalar(Array<T> A, T x) {
  int count = A.size();
  Array<T> B(count);
  T *a = A.ptr();
  T *b = B.ptr();
  for (int i = 0; i < count; i++) {
    b[i] = a[i] + x;
  }
  return B;
}

int countSliceSize(int count, SliceFun fun);
int countSlicedElements(Arrayi arr, SliceFun fun);
Arrayi makeIndexMap(int oldCount, SliceFun fun);
int indexOf(int index, Arrayi inds);
Arrayi makeRange(int count);
int countTrue(Arrayb array);
Arrayb ind2log(int len, Arrayi inds);
Arrayb neg(Arrayb X);
bool all(Arrayb X);
bool any(Arrayb X);
Arrayi makeSparseInds(int arraySize, int sampleCount);

} /* namespace sail */
#endif /* ARRAY_H_ */
