/*
 *  Created on: 28 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef JSON_H_
#define JSON_H_

#include <Poco/JSON/Object.h>

namespace sail {
namespace json {

class MissingFieldException {
 public:
  MissingFieldException(Poco::JSON::Object::Ptr obj, std::string field) :
    _obj(obj), _field(field) {}
 private:
  Poco::JSON::Object::Ptr _obj;
  std::string _field;
};

}
}



#endif /* JSON_H_ */
