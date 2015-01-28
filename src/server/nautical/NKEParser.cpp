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
    std::string alt1 = " ";
    alt1[0] = 176;

    std::string alt2 = "   ";
    alt2[0] = 239;
    alt2[1] = 191;
    alt2[2] = 189;

    return (s.find(alt1) != s.npos || s.find(alt2) != s.npos);

//    for (int i = 0; i < s.length(); i++) {
//
//      // The degree sign in the text encoding used for the CSV exported by LogConverter
//      // has this code.
//      if ((unsigned char)(s[i]) == 176) {
//        return true;
//      }
//    }
//    return false;
  }
}


std::shared_ptr<NKEUnit> NKEUnit::make(const std::string &key) {

  if (hasDegreeSign(key)) { // For some reason, the degree sign ° is sometimes suffixed
                                   // by a character, e.g. V, such as °V
                                   // for CapFondMes.
    return std::shared_ptr<NKEUnit>(new NKEAngleUnit(Angle<double>::degrees(1.0)));
  } else if (key == "Nd") { // Noeuds
    return std::shared_ptr<NKEUnit>(new NKEVelocityUnit(Velocity<double>::knots(1.0)));
  } else if (key == "") {
    return std::shared_ptr<NKEUnit>(new NKEDimensionlessUnit());
  }
  for (int i = 0; i < key.length(); i++) {
    LOG(INFO) << "Code of " << key[i] << " = " << (unsigned char)(key[i]);
  }
  LOG(FATAL) << "Unknown unit: " << key;
  return std::shared_ptr<NKEUnit>();
}

NKEArray::NKEArray(std::shared_ptr<NKEUnit> unit,
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

  /* Empty template for extending the list below.

    types.add(NKEType(
       , Array<std::string>{"", ""},
           ""
       ));

     */


   /*
    * Some questions to ask Hugo:
    *
    * - Does the suffix _MES mean that it is the raw value? Similarly, when
    *   it is not there, does it mean that the value has been corrected?
    *
    * - The suffix _PIL means auto pilot I believe. Is it a corrected value?
    *
    * To create a Nav, we will need:
    *
    * Apparent wind angle (AWA) (NKE calls it AVA: Angle Vent Apparrent)
    *   24, AncAVA_Pil
    *   60, AncAVA
    *   72, AngVentApp (GIRMP, la girouette) <-- Le plus brût?
    *   234, AVA_Cor
    *
    *   Quelle entre ces mésures est la plus brûte?
    *
    *   THIS IS THE RAW APPARENT WIND ANGLE:
    *   [HK]  à 193, AVA_MesHR = Valeur Brute
    *
    *
    * Apparent wind speed AWS (NKE calls it VVA: Vitesse Vent Apparent)
    *   59, VitVentApp (ANEMO) <-- Le plus brût?
    *   235, VVA_Cor
    *
    *   THIS IS THE RAW APPARENT WIND SPEED:
    *   192, VVA_MesHR
    *
    * Magnetic compass (NKE refers to it using Mag, Compas)
    *   25, CapMagPil <-- Corrigé?
    *   61, CapMag    <-- Plus brût
    *
    *   See also:
    *     229, DeclMag
    *
    *   Est-ce que les mésures 25 and 61 ont déja été corrigés par la déclinaison magnétique?
    *
    *   [HK] Cap Vrai canal 212
    *   Sinon le canal 267 est intéressant( 3DH_Lacet) c’est la valeur la plus réactive ( dans le référentiel magnétique)
    *
    *   This is the raw magnetic value:
    *   212, CapVrai
    *
    * Speedo, the water passing the boat (SPEEDO)
    *   58, VitesSurf   <-- Plus brût?
    *   230, VitSurfPil <-- Corrigé?
    *
    *
    *   [HK] Pour valeurs brutes voir 120 et 124
    *
    *
    *
    *
    * GPS bearing over ground:
    *   66, CapFond <-- Filtered.
    *   233, CapFondMes <-- Plus brût. Yes
    *
    * GPS speed over ground:
    *   65, VitFond
    *   232, VitFondMes <-- Plus brût. Yes.
    *
    *
    * GPS position:
    *   86, Latitude
    *   87, LatDecMin
    *   88, Longitude
    *   89, LonDecMin
    *
    *
    *
    * Inferred wind and current:
    *
    * What is the difference between VitVentFnd and VitVentRl?
    * See maybe:
    *   http://www.cruisersforum.com/forums/f121/differences-between-ground-apparent-and-true-wind-direction-73563.html
    *
    *
    * Wind:
    *   214, VitVentFnd
    *   213, DirVentFnd
    *
    *   37, VitVentRl
    *   38, AngVentRl
    *
    * Current:
    *   76, VitCourMes
    *   77, DirCourMes
    *
    */


  types.add(NKEType(
      24, Array<std::string>{"AncAVA_Pil", "R_ANG_VENT_APP"},
        "Apparent wind angle from auto pilot.",
        Array<std::string>{"angle", "wind"}
      ));
  types.add(NKEType(
        25, Array<std::string>{"CapMagPil", "R_COMPASS"},
        "Magnetic compass direction from auto pilot.",
        Array<std::string>{"angle", "boat"}
    ));

  types.add(NKEType(
          37, Array<std::string>{"VitVentRl", "VIT_VENT_VRAI"},
          "True wind speed.",
          Array<std::string>{"speed", "wind"}
      ));

  types.add(NKEType(
      38, Array<std::string>{"AngVentRl", "ANG_VENT_VRAI"},
          "True wind angle (angle w.r.t. boat)",
          Array<std::string>{"angle", "wind"}
      ));

  types.add(NKEType(
      39, Array<std::string>{"DVR_Mag", "DIR_VENT_VRAI"},
          "True wind direction (angle w.r.t. magnetic north)",
        Array<std::string>{"angle", "wind"}
      ));

  types.add(NKEType(
      58, Array<std::string>{"VitesSurf", "SPEEDO"},
          "Speed of water passing the boat.",
      Array<std::string>{"speed", "water", "boat"}
      ));

  types.add(NKEType(
      59, Array<std::string>{"VitVentApp", "ANEMO"},
          "Speed of apparent wind",
          Array<std::string>{"wind", "speed"}
      ));

  types.add(NKEType(
      60, Array<std::string>{"AncAVA", "ANG_VENT_APP"},
          "Angle of apparent wind.",
          Array<std::string>{"wind", "angle"}
      ));


  types.add(NKEType(
      61, Array<std::string>{"CapMag", "COMPAS"},
          "Magnetic compass.",
          Array<std::string>{"boat", "angle"}
      ));


  types.add(NKEType(
      65, Array<std::string>{"VitFond", "V_FOND"},
          "Speed over ground",
          Array<std::string>{"speed", "boat"}
      ));

  types.add(NKEType(
      66, Array<std::string>{"CapFond", "CAP_FOND"},
          "Course over ground"
      ));



  types.add(NKEType(
      72, Array<std::string>{"AngVentApp", "GIRMP"},
          "Apparent wind angle. See also AVA_Cor (234) and AncAVA (60) ",
          Array<std::string>{"wind", "angle"}
      ));

  types.add(NKEType(
      76, Array<std::string>{"VitCourMes", "V_COURANT"},
          "Speed of current.",
          Array<std::string>{"water", "speed"}
      ));

  types.add(NKEType(
      77, Array<std::string>{"DirCourMes", "C_COURANT"},
          "Direction of current",
          Array<std::string>{"water", "angle"}
      ));

  types.add(NKEType(
      86, Array<std::string>{"Latitude", "LAT_DEGMIN"},
          "Latitude in degrees and minutes",
          Array<std::string>{"boat", "position"}
      ));

  types.add(NKEType(
      87, Array<std::string>{"LatDecMin", "LAT_MILMIN"},
          "Latitude expressed as minutes? Or some fraction of a minute?",
          Array<std::string>{"boat", "position"}
      ));

  types.add(NKEType(
      88, Array<std::string>{"Longitude", "LON_DEGMIN"},
          "Longitude in degrees and minutes",
          Array<std::string>{"boat", "position"}
      ));

  types.add(NKEType(
      89, Array<std::string>{"LonDecMin", "LON_MILMIN"},
          "Longitude.",
          Array<std::string>{"boat", "position"}
      ));

  types.add(NKEType(
        120, Array<std::string>{"VitSurUsTr", "SPEEDO_US",
          "navRawWatSpeed"},
            "Raw speedo value"
        ));
  types.add(NKEType(
        124, Array<std::string>{"VitSurUsBa", "SPEEDO_US_BA"},
            "Raw speedo value"
        ));

  types.add(NKEType(
      192, Array<std::string>{"VVA_MesHR", "VVA_MESHR",
        "navRawAWS"},
      "This is the raw apparent wind speed."
  ));


  types.add(NKEType(
      193, Array<std::string>{"AVA_MesHR", "AVA_MESHR",
        "navRawAWA"},
      "This is the raw apparent wind angle."
  ));

  types.add(NKEType(
      212, Array<std::string>{"CapVrai", "CAP_VRAI",
      "navRawMagHdg"}, // <-- is it the magnetic heading of the boat. I think so.
      "The raw magnetic value."
  ));

  types.add(NKEType(
      213, Array<std::string>{"DirVentFnd", "DIR_VENT_FOND"},
          "Direction of wind over ground.",
          Array<std::string>{"wind", "angle"}
      ));

  types.add(NKEType(
      214, Array<std::string>{"VitVentFnd", "VIT_VENT_FOND"},
          "Wind speed over ground",
          Array<std::string>{"wind", "speed"}
      ));

  types.add(NKEType(
      227, Array<std::string>{"VVR_Pilote", "VVR_PILOTE"}, // vitesse vent reelle
          "True wind speed from the auto pilot.",
          Array<std::string>{"wind", "speed"}
      ));

  types.add(NKEType(
      228, Array<std::string>{"AVR_Pilote", "AVR_PILOTE"},
          "True wind angle from the auto pilot.",
          Array<std::string>{"wind", "angle"}
      ));


  types.add(NKEType(
      229, Array<std::string>{"DeclMag", "DECL_MAG"},
          "Magnetic declination",
          Array<std::string>{"angle"}
      ));

  types.add(NKEType(
      230, Array<std::string>{"VitSurfPil", "SPEEDO_PIL"},
          "Water surface speed from auto pilot.",
          Array<std::string>{"speed", "water", "boat"}
      ));

  types.add(NKEType(
      232, Array<std::string>{"VitFondMes", "V_FOND_MES",
              "navGpsSpeed"},
          "Measured boat speed.",
          Array<std::string>{"speed", "boat"}
      ));

  types.add(NKEType(
      233, Array<std::string>{"CapFondMes", "C_FOND_MES",
                              "navGpsBearing"},
          "Measured boat direction over ground (bearing). MEASURED?",
          Array<std::string>{"angle", "boat"}
      ));

  types.add(NKEType(
      234, Array<std::string>{"AVA_Cor", "AVA_COR"},
          "Corrected apparent wind angle. CORRECTED",
          Array<std::string>{"angle", "wind"}
      ));

  types.add(NKEType(
      235, Array<std::string>{"VVA_Cor", "VVA_COR"},
          "Corrected apparent wind speed. CORRECTED.",
          Array<std::string>{"speed", "wind"}
      ));


  types.add(NKEType(
      236, Array<std::string>{"Orig_VVR", "ORIG_VVR"},
          "Original true wind speed?",
          Array<std::string>{"wind", "speed"}
      ));

  types.add(NKEType(
      237, Array<std::string>{"Orig_AVR", "ORIG_AVR"},
          "Original true wind angle?",
          Array<std::string>{"angle", "boat"}
      ));

  types.add(NKEType(
      238, Array<std::string>{"Orig_DVR", "ORIG_DVR"},
          "Original true wind direction.",
          Array<std::string>{"wind", "angle"}
      ));
  return types.get();
}

NKEData::NKEData(TimeStamp offset, Arrayi typeIndices, Array<NKEArray> values) :
    _typeIndices(typeIndices),
    _values(values), _offset(offset) {
    assert(_typeIndices.size() == _values.size());
    for (int i = 0; i < _typeIndices.size(); i++) {
      _type2column[_typeIndices[i]] = i;
    }
}

Array<TimeStamp> NKEData::timeStamps() const {
  return _values[0].durations().map<TimeStamp>([&](const Duration<double> &d) {
    return d + _offset;
  });
}

namespace {
  int parseInt(const std::string &s) {
    std::stringstream ss;
    ss << s;
    int x;
    ss >> x;
    return x;
  }

  TimeStamp getTimeOffsetFromFilename(const std::string &s) {
    int n = s.length();
    assert(s.substr(n - 4, 4) == ".csv");
    return TimeStamp::date(parseInt(s.substr(6, 4)),
                           parseInt(s.substr(3, 2)),
                           parseInt(s.substr(0, 2)));
  }
}

NKEData NKEParser::load(const std::string filename) {
  std::ifstream file(filename);
  LOG(INFO) << "Load file " << filename;
  return load(getTimeOffsetFromFilename(filename), file);
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

  std::shared_ptr<NKEUnit> getUnit(std::string token) {
    int from = token.find("(");
    if (from != token.npos) {
      int to = token.find(")");
      assert(from < to);
      LOG(INFO) << "Get unit from token " << token;
      return NKEUnit::make(trim(token.substr(from+1, to - from - 1)));
    }
    return std::shared_ptr<NKEUnit>();
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

NKEData NKEParser::load(TimeStamp offset, std::istream &file) {
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
      LOG(WARNING) << "This type (" << name <<
          ") is not recognized. Please register it in the code \n" <<
          "in the function makeNKETypes in the file " << __FILE__ << ".";
    }

    auto unit = (i == 0?
                  std::shared_ptr<NKEUnit>(new NKETimeOfDayUnit()) : // <-- No unit annotation for Date_Time.
                  getUnit(header));

    values[i] = NKEArray(unit, table.sliceCol(i).getStorage().slice(1, rows-1));
  }
  return NKEData(offset, typeInds, values);
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


}
