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
 *
 * The idea is to provide a unified json interface for all types of interest on the form
 *
 *    CommonJson::Ptr sail::json::serialize(const type &x);
 *    void sail::json::deserialize(CommonJson::Ptr src, type *dst);
 *
 * facilitating writing generic code.
 */
class CommonJsonVar;
class CommonJsonArray;
class CommonJsonObject;
class CommonJson {
 public:
  typedef std::shared_ptr<CommonJson> Ptr;
  enum SubType {Var, Array, Object};

  virtual bool isDynamicVar() const {return false;}
  virtual bool isArray() const {return false;}
  virtual bool isObject() const {return false;}

  // Please first check with isX() before any of these methods is called
  // if you want to avoid an error.
  virtual CommonJsonVar *toVar() {invalid(); return nullptr;}
  virtual CommonJsonArray *toArray() {invalid(); return nullptr;}
  virtual CommonJsonObject *toObject() {invalid(); return nullptr;}



  virtual void addToOtherArray(Poco::JSON::Array *dst) = 0;


  static CommonJson::Ptr getOtherArrayElement(Poco::JSON::Array &src, int index);
  static CommonJson::Ptr getOtherArrayElement(Poco::JSON::Array::Ptr src, int index);

  virtual void stringify(std::ostream& out, unsigned int indent = 0, int step = -1) const = 0;

  virtual ~CommonJson() {}


  // Approximately what the method below does: dst.setField(fieldName, this)
  virtual void setOtherObjectField(Poco::JSON::Object::Ptr dst,
      const std::string &fieldName) = 0;

  // Approximately what the static method below does: return src.getField(fieldName)
  static CommonJson::Ptr getOtherObjectField(Poco::JSON::Object::Ptr src,
      const std::string &fieldName);

 private:
  void invalid() const;
};

class CommonJsonVar : public CommonJson {
 public:
  CommonJsonVar(Poco::Dynamic::Var x) : _x(x) {}
  Poco::Dynamic::Var &get() {return _x;}
  bool isDynamicVar() const {return true;}
  CommonJsonVar *toVar() {return this;}
  void addToOtherArray(Poco::JSON::Array *dst);
  void setOtherObjectField(Poco::JSON::Object::Ptr dst, const std::string &fieldName);
  void stringify(std::ostream& out, unsigned int indent = 0, int step = -1) const;
 private:
  Poco::Dynamic::Var _x;
};

class CommonJsonArray : public CommonJson {
 public:
  CommonJsonArray(Poco::JSON::Array x) : _x(new Poco::JSON::Array(x)) {}
  CommonJsonArray(Poco::JSON::Array::Ptr x) : _x(x) {}
  CommonJsonArray();
  static CommonJson::Ptr make();

  Poco::JSON::Array::Ptr &get() {return _x;}
  bool isArray() const {return true;}
  CommonJsonArray *toArray() {return this;}
  void addToOtherArray(Poco::JSON::Array *dst);
  void setOtherObjectField(Poco::JSON::Object::Ptr dst, const std::string &fieldName);
  void stringify(std::ostream& out, unsigned int indent = 0, int step = -1) const;

  CommonJson::Ptr get(int index) const;
  void add(CommonJson::Ptr obj);
 private:
  Poco::JSON::Array::Ptr _x;
};

class CommonJsonObject : public CommonJson {
 public:
  static CommonJson::Ptr make();
  CommonJsonObject();
  CommonJsonObject(Poco::JSON::Object::Ptr x) : _x(x) {}
  Poco::JSON::Object::Ptr &get() {return _x;}
  bool isObject() const {return true;}
  CommonJsonObject *toObject() {return this;}
  void addToOtherArray(Poco::JSON::Array *dst);
  void setOtherObjectField(Poco::JSON::Object::Ptr dst, const std::string &fieldName);
  void stringify(std::ostream& out, unsigned int indent = 0, int step = -1) const;

  CommonJson::Ptr get(const std::string &key) const;
  void set(const std::string &key, CommonJson::Ptr obj);
 private:
  Poco::JSON::Object::Ptr _x;
};

}
} /* namespace sail */

#endif /* COMMONJSON_H_ */
