#ifndef ARRAY_H_
#define ARRAY_H_

#include <assert.h>
#include <functional>
#include <server/common/invalidate.h>

namespace sail {

#ifndef SAFEARRAY
#define SAFEARRAY 1
#endif

typedef std::function<bool(int)> SliceFun;

template <typename T>
class Deallocator {
 public:
  Deallocator() {}
  virtual void deallocate(int *refs, T *base) = 0;
  virtual ~Deallocator() {}
};

namespace ArrayAlloc {

  template <typename T>
  class InitArray {
   public:
    static void apply(int n, T *x) {}
  };

  #define INVALIDATE_DATA_FOR_TYPE(T) template <> class InitArray<T> {public: static void apply(int n, T *x) {InvalidateScalars<T>(n, x);}};
  INVALIDATE_DATA_FOR_TYPE(double)
  INVALIDATE_DATA_FOR_TYPE(float)
  INVALIDATE_DATA_FOR_TYPE(int)
  INVALIDATE_DATA_FOR_TYPE(unsigned int)
  INVALIDATE_DATA_FOR_TYPE(char)
  INVALIDATE_DATA_FOR_TYPE(unsigned char)
  INVALIDATE_DATA_FOR_TYPE(long)
  INVALIDATE_DATA_FOR_TYPE(unsigned long)
  INVALIDATE_DATA_FOR_TYPE(short)
  INVALIDATE_DATA_FOR_TYPE(unsigned short)
  #undef INVALIDATE_DATA_FOR_TYPE



  template <typename T>
  T *make(int n) {
    T *data = new T[n];
    InitArray<T>::apply(n, data);
    return data;
  }
}


// Implements a reference counted ("garbage collected") and sliceable generic array type.
template <typename T>
class Array {
 public:
  typedef Array<T> ThisType;
  typedef T ElemType;

  Array() {
    _size = 0;
    _refs = nullptr;
    _data = nullptr;
    _base = nullptr;
    _deallocator = nullptr;
  }

  typedef std::function<T(int index)> ArrayFun;
  static ThisType fill(int size, ArrayFun fun) {
    ThisType dst(size);
    for (int i = 0; i < size; i++) {
      dst.set(i, fun(i));
    }
    return dst;
  }

  static ThisType args(T a) {
    ThisType dst(1);
    dst[0] = a;
    return dst;
  }

  static ThisType args(T a, T b) {
    ThisType dst(2);
    dst[0] = a;
    dst[1] = b;
    return dst;
  }

  static ThisType args(T a, T b, T c) {
    ThisType dst(3);
    dst[0] = a;
    dst[1] = b;
    dst[2] = c;
    return dst;
  }

  static ThisType args(T a, T b, T c, T d) {
    ThisType dst(4);
    dst[0] = a;
    dst[1] = b;
    dst[2] = c;
    dst[3] = d;
    return dst;
  }

  static ThisType args(T a, T b, T c, T d, T e) {
    ThisType dst(5);
    dst[0] = a;
    dst[1] = b;
    dst[2] = c;
    dst[3] = d;
    dst[4] = e;
    return dst;
  }

  static ThisType args(T a, T b, T c, T d, T e, T f) {
    ThisType dst(6);
    dst[0] = a;
    dst[1] = b;
    dst[2] = c;
    dst[3] = d;
    dst[4] = e;
    dst[5] = f;
    return dst;
  }

  void destructure(T &a) {
    a = get(0);
  }

  void destructure(T &a, T &b) {
    a = get(0);
    b = get(1);
  }

  void destructure(T &a, T &b, T &c) {
    a = get(0);
    b = get(1);
    c = get(2);
  }

  void destructure(T &a, T &b, T &c, T &d) {
    a = get(0);
    b = get(1);
    c = get(2);
    d = get(3);
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

  static ThisType single(T x) {
    ThisType dst(1);
    dst.set(0, x);
    return dst;
  }

  template <typename S>
  static ThisType referToVector(S &x) {
    return ThisType(x.size(), x.data());
  }

  bool empty() {
    return _size == 0;
  }

  bool hasData() {
    return not empty();
  }

  // Create a new array of specified size
  Array(int size) {
#if SAFEARRAY
    assert(0 <= size);
#endif
    _size = size;
    _refs = new int;
    *_refs = 1;
    _base = ArrayAlloc::make<T>(size);
    _data = _base;
    _deallocator = nullptr;
  }

  void copyToSafe(Array<T> dst) {
    assert(dst.size() == _size);
    for (int i = 0; i < _size; i++) {
      dst[i] = _data[i];
    }
  }


  void assignOwnage(int size, T *data) {
    decRef();
    _size = size;
    _refs = new int;
    *_refs = 1;
    _base = data;
    _data = _base;
  }

  void create(int size) {
#if SAFEARRAY
    assert(0 <= size);
#endif
    if (_size != size) {
      assignOwnage(size, ArrayAlloc::make<T>(size));
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
      T *newData = ArrayAlloc::make<T>(newSize);
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

  template <typename S>
  static ThisType make(S &value) {
    int count = sizeof(S);
    T *data = (T *)(&value);
    return makeCopy(count, data);
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

  ThisType dup() {
    int count = size();
    ThisType dst(count);
    for (int i = 0; i < count; i++) {
      dst.set(i, get(i));
    }
    return dst;
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
  Array(int size, T *data, int *refs = nullptr, Deallocator<T> *deallo = nullptr) {
#if SAFEARRAY
    assert(data != nullptr || size == 0);
    assert(0 <= size);
#endif
    _size = size;
    _base = data;
    _refs = refs;
    _data = data;
    _deallocator = deallo;
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
    int count = _size;
    for (int i = 0; i < _size; i++) {
      _data[i] = src.get(i);
    }

  }

  // Get a slice of size to - from
  ThisType slice(int from, int to) {
    incRef(); // Increase for the slice
#if SAFEARRAY
    assert(from <= to);
    assert(0 <= from);
    assert(to <= _size);
    assert(_data != nullptr);
#endif
    return Array(_refs, _base, _data + from, to - from, _deallocator);
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

  ThisType slice(std::function<bool(int index)> fun) {
    int counter = 0;
    ThisType Y(_size);
    for (int i = 0; i < _size; i++) {
      if (fun(i)) {
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

  ThisType sliceFrom(int from) {
    return slice(from, _size);
  }

  ThisType sliceTo(int to) {
    return slice(0, to);
  }

  ThisType sliceLast(int n) {
    return sliceFrom(_size - n);
  }

  ThisType sliceBut(int n) {
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

  // Copy constructor
  Array(const ThisType &other) {
    _size = other._size;
    _base = other._base;
    _data = other._data;
    _refs = other._refs;
    _deallocator = other._deallocator;
    incRef();
  }

  ThisType &operator=(const ThisType &other) {
    if (_refs != other._refs) {
      decRef();
      _refs = other._refs;
      incRef();
    }
    _size = other._size;
    _base = other._base;
    _data = other._data;
    _deallocator = other._deallocator;
    return *this;
  }

  bool operator==(const ThisType &other) {
    if (other.size() != _size) {
      return false;
    }
    for (int i = 0; i < _size; i++) {
      if (_data[i] != other._data[i]) {
        return false;
      }
    }

    return true;
  }

  bool byteEquals(const ThisType &other) {
    return cast<unsigned char>() == other.cast<unsigned char>();
  }

  ThisType &operator=(T *src) {
    assert(src == nullptr);
    decRef();
    _refs = nullptr;
    _base = src;
    _data = src;
    if (src == nullptr) {
      _size = 0;
    }
    return *this;
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

  virtual ~Array() {
    decRef();
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

  int getRefs() {
    if (_refs == nullptr) {
      return -1;
    } else {
      return *_refs;
    }
  }

  template <typename S>
  S reduce(S init, std::function<S(S, T)> red) {
    assert(_size >= 2);
    S x = init;
    for (int i = 0; i < _size; i++) {
      x = red(x, _data[i]);
    }
    return x;
  }

  template <typename S>
  Array<S> map(std::function<S(T)> mapper) {
    Array<S> dst(_size);
    for (int i = 0; i < _size; i++) {
      dst[i] = mapper(_data[i]);
    }
    return dst;
  }

  template <typename S>
  Array<S> mapi(std::function<S(int, T)> mapper) {
    Array<S> dst(_size);
    for (int i = 0; i < _size; i++) {
      dst[i] = mapper(i, _data[i]);
    }
    return dst;
  }

  template <typename S>
  Array<S> mapElements(std::function<S(T)> mapper) { // In case of mixup with std::map
    return map<S>(mapper);
  }

  T* getBase() {
    return _base;
  }

  int* getRefCounter() {
    return _refs;
  }

  void incRef() {
    if (_refs != nullptr) {
      (*_refs)++;
    }
  }

  // A constructor that gives full control to initialization of all the five fields
  Array(int *refs, T *base, T *data, int size, Deallocator<T> *deallocator) {
    _refs = refs;
    _base = base;
    _data = data;
    _size = size;
    _deallocator = deallocator;
  }

  T &last() {
    return _data[_size - 1];
  }

  T &first() {
    return _data[0];
  }

  bool identicTo(const Array<T> &other) {
    return _size == other._size  && _data == other._data && _base == other._base && other._refs == _refs && other._deallocator == _deallocator;
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
 private:

  void decRef() {
    if (_refs != nullptr) {
      (*_refs)--;
      if ((*_refs) == 0) {
        if (_deallocator == nullptr) {
          delete _refs;
          delete[] _base;
        } else {
          _deallocator->deallocate(_refs, _base);
        }
        _refs = nullptr;
        _base = nullptr;
        _data = nullptr;
      }
    }
  }

  Deallocator<T> *_deallocator;
  int *_refs; // A reference counter
  T *_base;   // A pointer to the beginning of the allocated memory, to use for deallocation
  T*_data;    // A pointer to the start of the data
  int _size;  // The size of the array
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
