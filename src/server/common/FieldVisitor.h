/*
 * FieldVisitor.h
 *
 *  Created on: 18 Aug 2017
 *      Author: jonas
 */

#ifndef SERVER_COMMON_FIELDVISITOR_H_
#define SERVER_COMMON_FIELDVISITOR_H_

namespace sail {

// Used to decorate a visitor with various options
// that may, or may not, be relevant for your particular visitor.
// The idea is to that they work as an options or parameter map
// used when visiting a field.
//
// Common setting
struct FieldVisitorBase {
  // Whether subsequent fields are required to be
  // present in the log file.
  bool required = true;

  // You need to implement the 'visit' method:
  //
  // template <typename T>
  // void visit(const char* fieldName, T& x);
};

struct DummyVisitor : public FieldVisitorBase {
  template <typename T>
  void visit(const char* fieldName, T& x) {}
};

template <typename T, typename = void>
struct CanVisitFields {
  static const bool value = false;
};

template <typename T>
struct CanVisitFields<T, decltype(
    std::declval<T>()
      .template visitFields<DummyVisitor>(
          std::declval<DummyVisitor*>()))> {
  static const bool value = true;
};

}



#endif /* SERVER_COMMON_FIELDVISITOR_H_ */
