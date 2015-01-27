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

NKEUnit::Ptr NKEUnit::make(const std::string &key) {

  if (key.find("°") != key.npos) { // For some reason, the degree sign ° is sometimes suffixed
                                   // by a character, e.g. V, such as °V
                                   // for CapFondMes.
    return NKEUnit::Ptr(new NKEAngleUnit(Angle<double>::degrees(1.0)));
  } else if (key == "Nd") { // Noeuds
    return NKEUnit::Ptr(new NKEVelocityUnit(Velocity<double>::knots(1.0)));
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

NKEParser::NKEParser() {
  Array<NKEType> types = makeNKETypes();
  for (auto type : types) {
    for (auto name : type.names()) {
      _name2type[name] = type;
    }
  }
}


} /* namespace mmm */
