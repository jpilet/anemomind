/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef NKEPARSER_H_
#define NKEPARSER_H_

#include <server/common/Array.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <map>
#include <set>
#include <server/common/TimeStamp.h>
#include <server/nautical/Nav.h>
#include <server/common/Functional.h>

namespace sail {


class NKEUnit {
 public:
  Angle<double> toAngle(const std::string &value) const {
    return toAngle(toDouble(value));
  }

  Velocity<double> toVelocity(const std::string &value) const {
    return toVelocity(toDouble(value));
  }

  virtual Angle<double> toAngle(double x) const;
  virtual Velocity<double> toVelocity(double x) const;
  virtual Duration<double> toDuration(const std::string &x) const;

  virtual bool isDimensionless() const {return false;}
  virtual bool isAngle() const {return false;}
  virtual bool isVelocity() const {return false;}
  virtual bool isDuration() const {return false;}

  double toDouble(const std::string &value) const;

  static std::shared_ptr<NKEUnit> make(const std::string &key);
  virtual ~NKEUnit() {}
};

class NKEDimensionlessUnit : public NKEUnit {
 public:
  bool isDimensionless() const {return true;}
};

class NKEAngleUnit : public NKEUnit {
 public:
  NKEAngleUnit(Angle<double> x) : _unit(x) {}

  Angle<double> toAngle(double x) const {
    return x*_unit;
  }

  virtual bool isAngle() const {return true;}
 private:
  Angle<double> _unit;
};

class NKEVelocityUnit : public NKEUnit {
 public:
  NKEVelocityUnit(Velocity<double> x) : _unit(x) {}

  Velocity<double> toVelocity(double x) const {
    return x*_unit;
  }

  virtual bool isVelocity() const {return true;}
 private:
  Velocity<double> _unit;
};

class NKETimeOfDayUnit : public NKEUnit {
 public:
  NKETimeOfDayUnit() {}

  virtual bool isDuration() const {return true;}

  Duration<double> toDuration(const std::string &x) const;
};



// Represents an array of values loaded from an NKE file, associated with a unit, such as
// knots.
class NKEArray {
 public:
  NKEArray() {}
  NKEArray(std::shared_ptr<NKEUnit> unit,
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

  Array<Angle<double> > angles() const {
    return toArray(map([&](const std::string &x) {
      return _unit->toAngle(x);
    }, _values));
  }

  Array<Velocity<double> > velocities() const {
    return toArray(map([&](const std::string &x) {
      return _unit->toVelocity(x);
    }, _values));
  }

  Array<Duration<double> > durations() const {
    return toArray(map([&](const std::string &x) {
      return _unit->toDuration(x);
    }, _values));
  }

  std::shared_ptr<NKEUnit> unit() const {
    return _unit;
  }

 private:
  std::shared_ptr<NKEUnit> _unit;
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

  Array<std::string> keywords() const {
    return _keywords;
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
  NKEData(TimeStamp offset, Arrayi typeIndices, Array<NKEArray> values);

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

  NKEArray col(int index) const {
    return _values[index];
  }

  Array<TimeStamp> timeStamps() const;

  bool hasAllFields(std::initializer_list<NKEType> types);
 private:
  Arrayi _typeIndices;
  std::map<int, int> _type2column;
  Array<NKEArray> _values;
  TimeStamp _offset;
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
  NKEData load(const std::string filename, std::istream &file);
  NKEData load(TimeStamp offset, std::istream &file);

  Array<Nav> makeNavs(Nav::Id boatId, NKEData data);
 private:
  // Maps a name to an NKE type
  std::map<std::string, NKEType> _name2type;

  // Maps an index to an NKE type
  std::map<int, NKEType> _index2type;
};

}

#endif /* NKEPARSER_H_ */
