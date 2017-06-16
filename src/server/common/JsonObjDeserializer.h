/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Helper object to properly deserialize json data.
 *
 *  NOTE: It may be a bad idea to use this object in header files, e.g.
 *  together with templates, because it may cause some circular dependencies.
 */

#ifndef JSONOBJDESERIALIZER_H_
#define JSONOBJDESERIALIZER_H_

#include <assert.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Object.h>

namespace sail {
namespace json {

// BEFORE USING THIS CLASS, SEE THE NOTE IN THE HEADER!!!
class ObjDeserializer {
 public:
  ObjDeserializer(Poco::Dynamic::Var x);

  template <typename T>
  void get(const std::string &fieldName, T *dst) {
    if (_success) { // Successful so far, otherwise ignore this code
      try {
        _success = deserialize(_src->get(fieldName), dst);
      } catch (Poco::Exception &e) {
        _success = false;
      }
    }
  }

  // Methods to allow for a work-around when the wrong version of 'deserialize' is chosen:
  //
  // deser.registerSuccess(myDeserializationFunction(deser.get("field"), &dst))
  void registerSuccess(bool s) {
    if (!s) {
      _success = false;
    }
  }
  Poco::Dynamic::Var get(const std::string &s) const {
    return _src->get(s);
  }


  bool success() {_checked = true; return _success;}

  // Ensure that the value of success is checked before this object is destroyed.
  ~ObjDeserializer() {assert(_checked);}
 private:
  Poco::JSON::Object::Ptr _src;
  bool _success, _checked;
};


}
} /* namespace sail */

#endif /* JSONOBJDESERIALIZER_H_ */
