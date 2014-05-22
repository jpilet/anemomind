/*
 *  Created on: May 22, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef COMMONJSON_H_
#define COMMONJSON_H_

#include <Poco/JSON/Object.h>
#include <memory>

namespace sail {
namespace json {

/*
 * The following classes are intended to simplify
 * writing Json serialization routines.
 *
 * The problem
 * with the Poco json library is that fields in
 * objects and arrays can be either Array, Object or Var.
 * These classes wrap all these objects into a
 * CommonJson object.
 */
class CommonJsonVar;
class CommonJsonArray;
class CommonJsonObject;
class CommonJson {
 public:
  typedef std::shared_ptr<CommonJson> Ptr;
  virtual bool isDynamicVar() const {return false;}
  virtual bool isArray() const {return false;}
  virtual bool isObject() const {return false;}

  // Please first check with isX() before any of these methods is called
  // if you want to avoid an error.
  virtual CommonJsonVar *toVar() {invalid(); return nullptr;}
  virtual CommonJsonArray *toArray() {invalid(); return nullptr;}
  virtual CommonJsonObject *toObject() {invalid(); return nullptr;}



  virtual void addToArray(Poco::JSON::Array *dst) = 0;

  virtual void setObjectField(Poco::JSON::Object::Ptr dst,
      std::string fieldName) = 0;

  static CommonJson::Ptr getObjectField(Poco::JSON::Object::Ptr src,
      std::string fieldName);

  static CommonJson::Ptr getArrayElement(Poco::JSON::Array &src, int index);
  static CommonJson::Ptr getArrayElement(Poco::JSON::Array::Ptr src, int index);

  virtual void stringify(std::ostream& out, unsigned int indent = 0, int step = -1) const {
    invalid();
  }

  virtual ~CommonJson() {}
 private:
  void invalid() const;
};

class CommonJsonVar : public CommonJson {
 public:
  CommonJsonVar(Poco::Dynamic::Var x) : _x(x) {}
  Poco::Dynamic::Var &get() {return _x;}
  bool isDynamicVar() {return true;}
  CommonJsonVar *toVar() {return this;}
  void addToArray(Poco::JSON::Array *dst);
  void setObjectField(Poco::JSON::Object::Ptr dst, std::string fieldName);
 private:
  Poco::Dynamic::Var _x;
};

class CommonJsonArray : public CommonJson {
 public:
  CommonJsonArray(Poco::JSON::Array x) : _x(new Poco::JSON::Array(x)) {}
  CommonJsonArray(Poco::JSON::Array::Ptr x) : _x(x) {}

  Poco::JSON::Array::Ptr &get() {return _x;}
  bool isArray() {return true;}
  CommonJsonArray *toArray() {return this;}
  void addToArray(Poco::JSON::Array *dst);
  void setObjectField(Poco::JSON::Object::Ptr dst, std::string fieldName);
 private:
  // TODO: Unsure of what is best: Poco::JSON::Array::Ptr or simply Poco::JSON::Array?
  Poco::JSON::Array::Ptr _x;
};

class CommonJsonObject : public CommonJson {
 public:
  CommonJsonObject(Poco::JSON::Object::Ptr x) : _x(x) {}
  Poco::JSON::Object::Ptr &get() {return _x;}
  bool isObject() {return true;}
  CommonJsonObject *toObject() {return this;}
  void addToArray(Poco::JSON::Array *dst);
  void setObjectField(Poco::JSON::Object::Ptr dst, std::string fieldName);
 private:
  Poco::JSON::Object::Ptr _x;
};

}
} /* namespace sail */

#endif /* COMMONJSON_H_ */
