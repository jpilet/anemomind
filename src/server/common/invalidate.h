// julien.pilet@gmail.com, 2.2014

/*
 * The function InvalidateScalar is a way to declare that a variable will not
 * be read before it is written. In other words, its initial content does not
 * matter. InvalidateScalar will populate float values by NaN in release and
 * debug mode to make sure reading such a variable will get noticed.
 *
 * If it is only desirable that the value is initialized to NaN in debug mode
 * but left untouched in release mode, e.g. for performance reasons,
 * InvalidateScalarInDebug can be called.
 *
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

template <typename T> void InvalidateScalar(T *var) {
    *var = std::numeric_limits<T>::signaling_NaN();
}

#ifdef NDEBUG
template <typename T> void InvalidateScalarInDebug(T *) { }
#else
template <typename T> void InvalidateScalarInDebug(T *var) {
    InvalidateScalar<T>(var);
}
#endif

#endif // COMMON_INVALIDATE_H
