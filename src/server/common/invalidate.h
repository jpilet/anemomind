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
#else
template <typename T> void InvalidateScalar(T *var) {
    *var = std::numeric_limits<T>::signaling_NaN();
}
#endif

#endif // COMMON_INVALIDATE_H
