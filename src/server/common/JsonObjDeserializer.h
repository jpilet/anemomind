/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Helper object to properly deserialize json data.
 */

#ifndef JSONOBJDESERIALIZER_H_
#define JSONOBJDESERIALIZER_H_

#include <assert.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Object.h>

namespace sail {
namespace json {

class ObjDeserializer {
 public:
  ObjDeserializer(Poco::Dynamic::Var x);

  template <typename T>
  void deserialize(const std::string &fieldName, T *dst) {
    if (_success) { // Successful so far, otherwise ignore this code
      try {
        _success = deserialize(_src->get(fieldName), dst);
      } catch (Poco::Exception &e) {
        _success = false;
      }
    }
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
