/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/NKEParser.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/logging.h>
#include <Poco/String.h>
#include <Poco/StringTokenizer.h>
#include <server/common/string.h>
#include <server/common/MDArray.h>
#include <fstream>

namespace sail {

using Poco::trim;
using Poco::StringTokenizer;


Angle<double> NKEUnit::toAngle(double x) const {
  LOG(FATAL) << "Not implemented";
  return Angle<double>::radians(NAN);
}

Velocity<double> NKEUnit::toVelocity(double x) const {
  LOG(FATAL) << "Not implemented";
  return Velocity<double>::knots(NAN);
}

Duration<double> NKEUnit::toDuration(const std::string &x) const {
  LOG(FATAL) << "Not implemented";
  return Duration<double>::seconds(NAN);
}

double NKEUnit::toDouble(const std::string &value) const {
  double result = 0.0;
  try {
    std::stringstream ss;
    ss << value;
    ss >> result;
    return result;
  } catch (std::exception &e) {
    LOG(FATAL) << stringFormat("Failed to convert '%s' to double", value.c_str());
    return NAN;
  }
}

namespace {
  bool hasDegreeSign(const std::string &s) {
    for (int i = 0; i < s.length(); i++) {

      // The degree sign in the text encoding used for the CSV exported by LogConverter
      // has this code.
      if ((unsigned char)(s[i]) == 176) {
        return true;
      }
    }
    return false;
  }
}


NKEUnit::Ptr NKEUnit::make(const std::string &key) {

  if (hasDegreeSign(key)) { // For some reason, the degree sign ° is sometimes suffixed
                                   // by a character, e.g. V, such as °V
                                   // for CapFondMes.
    return NKEUnit::Ptr(new NKEAngleUnit(Angle<double>::degrees(1.0)));
  } else if (key == "Nd") { // Noeuds
    return NKEUnit::Ptr(new NKEVelocityUnit(Velocity<double>::knots(1.0)));
  }
  for (int i = 0; i < key.length(); i++) {
    LOG(INFO) << "Code of " << key[i] << " = " << (unsigned char)(key[i]);
  }
  LOG(FATAL) << stringFormat("Unknown unit: '%s'", key.c_str());
  return NKEUnit::Ptr();
}

NKEArray::NKEArray(NKEUnit::Ptr unit,
    Array<std::string> values) :
    _unit(unit), _values(values) {}




NKEType::NKEType(int index, Array<std::string> names,
    std::string desc,
    Array<std::string> keywords) :
    _index(index), _names(names), _description(desc),
    _keywords(keywords) {}

Array<NKEType> makeNKETypes() {
  ArrayBuilder<NKEType> types;

  // The first column of the CSV data is always Date_Time,
  // no matter what data we choose to export from LogConverter.
  // I think it makes sense to treat this column as any other column,
  // so therefore I assign it its own NKEType.
  types.add(NKEType(
      -2, Array<std::string>{"Date_Time"},
      "Time of the day (hours:minutes:seconds)"
  ));



  types.add(NKEType(
      24, Array<std::string>{"AncAVA_Pil", "R_ANG_VENT_APP"},
        "Apparent wind angle from auto pilot."
      ));
  types.add(NKEType(
        25, Array<std::string>{"CapMagPil", "R_COMPASS"},
        "Magnetic compass direction from auto pilot."
    ));

  types.add(NKEType(
          37, Array<std::string>{"VitVentRl", "VIT_VENT_VRAI"},
          "True wind speed."
      ));

  types.add(NKEType(
      38, Array<std::string>{"AngVentRl", "ANG_VENT_VRAI"},
          "True wind angle (angle w.r.t. boat)"
      ));

  types.add(NKEType(
      39, Array<std::string>{"DVR_Mag", "DIR_VENT_VRAI"},
          "True wind direction (angle w.r.t. magnetic north)"
      ));

  types.add(NKEType(
      58, Array<std::string>{"VitesSurf", "SPEEDO"},
          "Speed of water passing the boat."
      ));

  types.add(NKEType(
      59, Array<std::string>{"VitVentApp", "ANEMO"},
          "Speed of apparent wind"
      ));

  types.add(NKEType(
      60, Array<std::string>{"AncAVA", "ANG_VENT_APP"},
          "Angle of apparent wind."
      ));


  types.add(NKEType(
      61, Array<std::string>{"CapMag", "COMPAS"},
          "Magnetic compass."
      ));

  types.add(NKEType(
      72, Array<std::string>{"AngVentApp", "GIRMP"},
          "Apparent wind angle."
      ));

  types.add(NKEType(
      76, Array<std::string>{"VitCourMes", "V_COURANT"},
          "Speed of current."
      ));

  types.add(NKEType(
      77, Array<std::string>{"DirCourMes", "C_COURANT"},
          "Direction of current"
      ));

  types.add(NKEType(
      86, Array<std::string>{"Latitude", "LAT_DEGMIN"},
          "Latitude in degrees and minutes"
      ));

  types.add(NKEType(
      87, Array<std::string>{"LatDecMin", "LAT_MILMIN"},
          "Latitude expressed as minutes? Or some fraction of a minute?"
      ));

  types.add(NKEType(
      88, Array<std::string>{"Longitude", "LON_DEGMIN"},
          "Longitude in degrees and minutes"
      ));

  types.add(NKEType(
      89, Array<std::string>{"LonDecMin", "LON_MILMIN"},
          "Longitude."
      ));


  types.add(NKEType(
      227, Array<std::string>{"VVR_Pilote", "VVR_PILOTE"},
          "True wind speed from the auto pilot."
      ));

  types.add(NKEType(
      228, Array<std::string>{"AVR_Pilote", "AVR_PILOTE"},
          "True wind angle from the auto pilot."
      ));


  types.add(NKEType(
      229, Array<std::string>{"DeclMag", "DECL_MAG"},
          "Magnetic declination"
      ));

  types.add(NKEType(
      230, Array<std::string>{"VitSurfPil", "SPEEDO_PIL"},
          "Water surface speed from auto pilot."
      ));

  types.add(NKEType(
      234, Array<std::string>{"AVA_Cor", "AVA_COR"},
          "Corrected apparent wind angle."
      ));

  types.add(NKEType(
      235, Array<std::string>{"VVA_Cor", "VVA_COR"},
          "Corrected apparent wind speed."
      ));


  types.add(NKEType(
      236, Array<std::string>{"Orig_VVR", "ORIG_VVR"},
          "Original true wind speed?"
      ));

  types.add(NKEType(
      237, Array<std::string>{"Orig_AVR", "ORIG_AVR"},
          "Original true wind angle?"
      ));

  types.add(NKEType(
      238, Array<std::string>{"Orig_DVR", "ORIG_DVR"},
          "Original true wind direction."
      ));
  return types.get();
}

NKEData::NKEData(Arrayi typeIndices, Array<NKEArray> values) :
    _typeIndices(typeIndices),
    _values(values) {
    assert(_typeIndices.size() == _values.size());
    for (int i = 0; i < _typeIndices.size(); i++) {
      _type2column[_typeIndices[i]] = i;
    }
}

NKEData NKEParser::load(const std::string filename) {
  std::ifstream file(filename);
  LOG(INFO) << "Load file " << filename;
  return load(file);
}

namespace {
  std::string getLine(std::istream &file) {
    return "";
  }

  std::string getName(std::string token) {
    int index = token.find("(");
    if (index == token.npos) {
      return token;
    }
    return trim(token.substr(0, index));
  }

  NKEUnit::Ptr getUnit(std::string token) {
    int from = token.find("(");
    if (from != token.npos) {
      int to = token.find(")");
      assert(from < to);
      LOG(INFO) << "Get unit from token " << token;
      return NKEUnit::make(trim(token.substr(from+1, to - from - 1)));
    }
    return NKEUnit::Ptr();
  }

  MDArray<std::string, 2> readCsv(std::istream &file) {
    ArrayBuilder<std::string> linesBuilder;
    std::string line;
    while (std::getline(file, line)) {
      linesBuilder.add(line);
    }
    LOG(INFO) << "LINE: " << line;

    Array<std::string> lines = linesBuilder.get();

    int count = lines.size();
    MDArray<std::string, 2> result;
    for (int i = 0; i < count; i++) {
      StringTokenizer tok(lines[i], ";",
          StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);

      if (result.empty()) {
        result = MDArray<std::string, 2>(count, tok.count());
      } else if (result.cols() != tok.count()) {
        return result.sliceRowsTo(i);
      }
      for (int j = 0; j < tok.count(); j++) {
        result(i, j) = tok[j];
      }
    }

    return result;
  }

  int parseInt(const std::string &s) {
    std::stringstream ss;
    ss << s;
    int x;
    ss >> x;
    return x;
  }

  // Quick-and-dirty parsing of time on the format
  // hh:mm:ss
  // which is the format of NKE. Returns a duration
  // since the start of the day (00:00:00).
  // This duration can be added to a timestamp when the day starts.
  Duration<double> parseTimeOfDay(std::string s) {
    assert(s.length() == 8);
    assert(s[2] == ':');
    assert(s[5] == ':');
    return Duration<double>::hours(parseInt(s.substr(0, 2))) +
           Duration<double>::minutes(parseInt(s.substr(3, 5))) +
           Duration<double>::seconds(parseInt(s.substr(6, 8)));
  }

}

Duration<double> NKETimeOfDayUnit::toDuration(const std::string &x) const {
  return parseTimeOfDay(x);
}

NKEData NKEParser::load(std::istream &file) {
  MDArray<std::string, 2> table = readCsv(file);
  int cols = table.cols();
  int rows = table.rows();
  assert(table(0, 0) == "Date_Time");

  Arrayi typeInds(cols);
  Array<NKEArray> values(cols);
  for (int i = 0; i < cols; i++) {
    const std::string &header = table(0, i);
    const std::string name = getName(header);
    auto type = _name2type[name];
    typeInds[i] = type.index();

    LOG(INFO) << "Type of column " << i << " (" << name << ") is " << type.index();
    if (type.index() == -1) {
      LOG(WARNING) << "This type is not recognized. Please add it.";
    }

    auto unit = (i == 0?
                  NKEUnit::Ptr(new NKETimeOfDayUnit()) : // <-- No unit annotation for Date_Time.
                  getUnit(header));

    values[i] = NKEArray(unit, table.sliceCol(i).getStorage().slice(1, rows-1));
  }
  return NKEData(typeInds, values);
}


NKEParser::NKEParser() {
  Array<NKEType> types = makeNKETypes();
  for (auto type : types) {
    _index2type[type.index()] = type;
    for (auto name : type.names()) {
      _name2type[name] = type;
    }
  }
}


} /* namespace mmm */
