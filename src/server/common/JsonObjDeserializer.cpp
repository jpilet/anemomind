/*
 *  Created on: Jun 17, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "JsonObjDeserializer.h"

namespace sail {
namespace json {

ObjDeserializer::ObjDeserializer(Poco::Dynamic::Var x) : _checked(false) {
  try {
    _src = x.extract<Poco::JSON::Object::Ptr>();
    _success = true;
  } catch (Poco::Exception &e) {
    _success = false;
  }
}


}
} /* namespace sail */
