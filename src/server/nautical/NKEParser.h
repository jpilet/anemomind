/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef NKEPARSER_H_
#define NKEPARSER_H_

#include <server/common/Array.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <map>

namespace sail {

//class NKEData {
// public:
//
// private:
//  // Maps an integer, representing an NKE code, to a column index.
//  std::map<int, int> _code2column;
//  MDArray<std::string, 2> _data;
//};


class NKEUnit {
 public:
  typedef std::shared_ptr<NKEUnit> Ptr;

  Angle<double> toAngle(const std::string &value) const {
    return toAngle(toDouble(value));
  }

  Velocity<double> toVelocity(const std::string &value) const {
    return toVelocity(toDouble(value));
  }

  virtual Angle<double> toAngle(double x) const;
  virtual Velocity<double> toVelocity(double x) const;
  virtual Duration<double> toDuration(const std::string &x) const;

  double toDouble(const std::string &value) const;

  static NKEUnit::Ptr make(const std::string &key);
  virtual ~NKEUnit() {}
};

class NKEAngleUnit : public NKEUnit {
 public:
  NKEAngleUnit(Angle<double> x) : _unit(x) {}

  Angle<double> toAngle(double x) const {
    return x*_unit;
  }
 private:
  Angle<double> _unit;
};

class NKEVelocityUnit : public NKEUnit {
 public:
  NKEVelocityUnit(Velocity<double> x) : _unit(x) {}

  Velocity<double> toVelocity(double x) const {
    return x*_unit;
  }
 private:
  Velocity<double> _unit;
};

class NKETimeOfDayUnit : public NKEUnit {
 public:
  NKETimeOfDayUnit() {}

  Duration<double> toDuration(const std::string &x) const;
};



// Represents an array of values loaded from an NKE file, associated with a unit, such as
// knots.
class NKEArray {
 public:
  NKEArray() {}
  NKEArray(NKEUnit::Ptr unit,
      Array<std::string> values);

  int size() const {
    return _values.size();
  }

  Angle<double> angle(int index) const {
    return _unit->toAngle(_values[index]);
  }

  Velocity<double> velocity(int index) const {
    return _unit->toVelocity(_values[index]);
  }

  Duration<double> duration(int index) const {
    return _unit->toDuration(_values[index]);
  }
 private:
  NKEUnit::Ptr _unit;
  Array<std::string> _values;
};


// Represents a type of NKE data, such as 'VitFondMes'
class NKEType {
 public:
  NKEType() : _index(-1) {}
  NKEType(int index, Array<std::string> names,
      std::string desc,
      Array<std::string> keywords = Array<std::string>());

  const Array<std::string> &names() const {
    return _names;
  }

  int index() const {
    return _index;
  }
 private:
  // Index given to this type by NKE
  int _index;

  // Names used to refer to this type
  Array<std::string> _names;

  // A description of what this type means.
  std::string _description;

  // Keywords so that types can be mapped to sets.
  // Just for helping
  Array<std::string> _keywords;
};


// Make all the NKE types that we are interested in.
Array<NKEType> makeNKETypes();


// A class to hold the loaded data from a csv file output
// from the LogConverter software of NKE.
class NKEData {
 public:
  NKEData(Arrayi typeIndices, Array<NKEArray> values);

  NKEArray getByType(const NKEType &type) {
    return _values[_type2column[type.index()]];
  }

  bool hasType(const NKEType &type) {
    return _type2column.find(type.index()) != _type2column.end();
  }

  int rows() const {
    return _values[0].size();
  }

  int cols() const {
    return _values.size();
  }
 private:
  Arrayi _typeIndices;
  std::map<int, int> _type2column;
  Array<NKEArray> _values;
};

class NKEParser {
 public:
  NKEParser();

  // Use this to the NKEType given one of its names.
  // The returned type can in turn be used to refer to one of the
  // loaded arrays in an object of the class NKEData, using its
  // 'getByType
  const NKEType &type(const std::string &name) {
    return _name2type[name];
  }

  const NKEType &type(int index) {
    return _index2type[index];
  }

  NKEData load(const std::string filename);
  NKEData load(std::istream &file);
 private:
  std::map<std::string, NKEType> _name2type;
  std::map<int, NKEType> _index2type;
};

}

#endif /* NKEPARSER_H_ */
