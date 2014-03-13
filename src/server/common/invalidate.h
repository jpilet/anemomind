// julien.pilet@gmail.com, 2.2014

/*
 * The function InvalidateScalar is a way to declare that a variable will not
 * be read before it is written. In other words, its initial content does not
 * matter. In debug mode, InvalidateScalar will populate float values by NaN,
 * to make sure reading such a variable will get noticed.
 * 
 * In release mode, the function does nothing.
 *
 * Usage example:
 *
 * struct Example {
 *   Example() { InvalidateScalar(&_member); }
 *   float _member;
 * };
 */
#ifndef COMMON_INVALIDATE_H
#define COMMON_INVALIDATE_H

#include <limits>

#ifdef NDEBUG
template <typename T> void InvalidateScalar(T *) { }
template <typename T> void InvalidateScalars(int, T *) { }
#else
template <typename T> void InvalidateScalar(T *var) {
    *var = std::numeric_limits<T>::signaling_NaN();
}
template <typename T> void InvalidateScalars(int n, T *var) {
  for (int i = 0; i < n; i++) {
    InvalidateScalar<T>(var + i);
  }
}
#endif

#endif // COMMON_INVALIDATE_H
