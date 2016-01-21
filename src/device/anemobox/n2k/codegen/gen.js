var parseString = require('xml2js').parseString;
var fs = require('fs');
var assert = require('assert');
var Path = require('path');

function makeSet() {
  return {};
}

function insertIntoSet(x, X) {
  X[x] = true; // Any dummy value that evaluates to true will do.
}

function makeSetFromArray(arr) {
  var dst = makeSet();
  for (var i = 0; i < arr.length; i++) {
    insertIntoSet(arr[i], dst);
  }
  return dst;
}

var pgnsOfInterest = makeSetFromArray(require('./pgnlist.json'));

function inSet(x, X) {
  return X[x] != undefined;
}

function makeWhitespace(depth) {
  var s = '';
  for (var i = 0; i < depth; i++) {
    s += '  ';
  }
  return s;
}

function beginLine(depth, n) {
  if (n == null || n == 0) {
    return "\n" + makeWhitespace(depth);
  } else {
    return "\n" + beginLine(depth, n-1);
  }

}

function getDuplicateId(pgns) {
  var idSet = makeSet();
  for (var i = 0; i < pgns.length; i++) {
    id = pgns[i].Id + '';
    if (inSet(id, idSet)) {
      return id;
    }
    insertIntoSet(id, idSet);
  }
  return null;
}

// http://stackoverflow.com/a/1026087
function capitalizeFirstLetter(string) {
    return string.charAt(0).toUpperCase() + string.slice(1);
}

function filterPgnsOfInterest(pgns) {
  return pgns.filter(function(x) {
    pgn = '' + x.PGN;
    return inSet(parseInt(pgn), pgnsOfInterest);
  });
}

function getPgnArrayFromParsedXml(xml) {
  var all = xml.PGNDefinitions.PGNs[0].PGNInfo;
  return filterPgnsOfInterest(all);
}

function getClassName(pgn) {
  return capitalizeFirstLetter(pgn.Id + '');
}

function getFieldArray(pgn) {
  assert(pgn.Fields.length == 1);
  var fields = pgn.Fields[0];
  var arr = fields.Field;
  assert(arr instanceof Array);
  return arr;
}

function makeClassBlock(name, publicDecls, privateDecls, depth) {
  return beginLine(depth, 1) + "class " + name + " {" 
    +beginLine(depth) + "public:"
    +publicDecls
    +beginLine(depth) + "private:"
    +privateDecls
    +beginLine(depth) + "};";
}

function getFieldId(field) {
  return '' + field.Id;
}

function makeInstanceVariableName(x) {
  return "_" + x;
}

function getInstanceVariableName(field) {
  return makeInstanceVariableName(getFieldId(field));
}

function getUnits(field) {
  return '' + field.Units;
}

function isPhysicalQuantity(field) {
  return field.Units != null;
}

function isSigned(field) {
  return field.Signed;
}

function getBitLength(field) {
  return parseInt(field.BitLength + "");
}

function makeIntegerReadExpr(field, srcName) {
  var extractor = isSigned(field)? "getSigned" : "getUnsigned";
  return srcName + "->" + extractor + "(" + getBitLength(field) + ")";
}

unitMap = {
  "m/s": {
    type: "sail::Velocity<double>",
    unit: "sail::Velocity<double>::metersPerSecond(1.0)",
  },
  "rad": {
    type: "sail::Angle<double>",
    unit: "sail::Angle<double>::radians(1.0)"
  }
};

function getUnitInfo(field) {
  var unit = getUnits(field);
  assert(unit in unitMap, "Unit not recognized: " +  unit);
  return unitMap[unit];
}


function getFieldType(field) {
  if (isPhysicalQuantity(field)) {
    return getUnitInfo(field).type;
  } else if (isSigned(field)) {
    return "int64_t";
  } else {
    return "uint64_t";
  }
}

function getAccessorName(field) {
  return "get" + capitalizeFirstLetter(getFieldId(field));
}

function makeFieldAccessor(field) {
  return "const " + getFieldType(field) + " &" 
    + getAccessorName(field) + "() const {assert(_valid); return "
    + getInstanceVariableName(field) + ";}";
}

function makeAccessors(pgn, depth) {
  var fields = getFieldArray(pgn);
  var s = beginLine(depth, 1) + "// Field access";
  for (var i = 0; i < fields.length; i++) {
    var field = fields[i];
    s += beginLine(depth) + makeFieldAccessor(field);
  }
  return s;
}

function makeCommonMethods(depth) {
  return beginLine(depth) + "bool valid() const {return _valid;}";
}

function makeDefaultConstructorDecl(pgn, depth) {
  return beginLine(depth) + getClassName(pgn) + "();";
}

function makeMethodsInClass(pgn, depth) {
  return makeDefaultConstructorDecl(pgn, depth)
    + makeConstructorDecl(pgn, depth) 
    + makeCommonMethods(depth) 
    + makeAccessors(pgn, depth);
}

function makeConstructorSignature(pgn) {
  return getClassName(pgn) + "(BitStream *src)";
}

function makeInstanceVariableDecls(pgn, depth) {
  var fields = getFieldArray(pgn);
  var commonDecls = beginLine(depth) + "bool _valid;";
  var s = commonDecls + beginLine(depth) + "// Number of fields: " + fields.length;
  for (var i = 0; i < fields.length; i++) {
    var field = fields[i];
    s += beginLine(depth) 
      + getFieldType(field) + " "
      + getInstanceVariableName(field) + ";";
  }
  return s;
}

function makeConstructorDecl(pgn, depth) {
  return beginLine(depth) + makeConstructorSignature(pgn) + ";";
}

function makeClassDeclarationFromPgn(pgn, depth) {
  var innerDepth = depth + 1;
  return makeClassBlock(
    getClassName(pgn), 
    makeMethodsInClass(pgn, innerDepth), 
    makeInstanceVariableDecls(pgn, innerDepth), 
    depth);
  return getClassName(pgn);
}


function wrapNamespace(label, data) {
  return "namespace " + label + " {" + data + "\n}";
}

function makeClassDeclarationsSub(pgns) {
  var depth = 1;
  var s = '';
  for (var i = 0; i < pgns.length; i++) {
      s += makeClassDeclarationFromPgn(pgns[i], depth);
  }
  return s;
}

function makeClassDeclarations(label, pgns) {
  return wrapNamespace(
    label, makeClassDeclarationsSub(pgns));
}

function wrapInclusionGuard(label, data) {
  var fullLabel = "_" + label + "_HEADER_";
  return "#ifndef " + fullLabel + "\n#define " + fullLabel + " 1\n\n" + data + "\n\n#endif";
}

function makeHeaderInclusion(moduleName) {
  return '#include "' + moduleName + '.h"\n\n';
}

function hasResolution(field) {
  return field.Resolution != null;
}

function makeFieldFromStreamExpr(field) {
  var unit = getUnits(field);
  var raw = makeIntegerReadExpr(field, "src");
  if (isPhysicalQuantity(field)) {
    var info = getUnitInfo(field);
    return "double(" + raw + 
      (hasResolution(field)? "*" + field.Resolution : "")
      + ")*" + info.unit;
  }
  return raw;
}

function getDefaultValue(field) {
  if (isPhysicalQuantity(field)) {
    return getUnitInfo(field).type + "()";
  }
  return "0";
}

function makeFieldAssignment(field) {
  return getInstanceVariableName(field) + " = " + makeFieldFromStreamExpr(field) + ";";
}

function getTotalBitLength(fields) {
  var n = 0;
  for (var i = 0; i < fields.length; i++) {
    var field = fields[i];
    n += parseInt(field.BitLength);
  }
  return n;
}

function makeConstructorStatements(pgn, depth) {
  var fields = getFieldArray(pgn);
  var s = beginLine(depth) + 
      'if (' + getTotalBitLength(fields) + ' <= src->remainingBits()) {';
  for (var i = 0; i < fields.length; i++) {
    s += beginLine(depth+1) + makeFieldAssignment(fields[i]);
  }
  return s + beginLine(depth) + "}";;
}


function makeConstructor(pgn, depth) {
  var innerDepth = depth + 1;
  return beginLine(depth) + getClassName(pgn) + "::" + makeConstructorSignature(pgn) 
    + " : _valid(false) {"
    + makeConstructorStatements(pgn, innerDepth)
    + beginLine(depth) + "}";
}

function makeMethodsForPgn(pgn, depth) {
  return makeConstructor(pgn, depth);
}

var privateInclusions = '#include <device/anemobox/n2k/BitStream.h>\n\n'

function makeImplementationFileContents(moduleName, pgns) {
  var depth = 1;
  var contents = "";
  for (var i = 0; i < pgns.length; i++) {
    contents += makeMethodsForPgn(pgns[i], depth);
  }
  return makeHeaderInclusion(moduleName) + privateInclusions + wrapNamespace(moduleName, contents);
}

var publicInclusions = '#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>\n'
    +'#include <cassert>\n'
    +'class BitStream;\n\n';

function makeInterfaceFileContents(moduleName, pgns) {
  return wrapInclusionGuard(
    moduleName.toUpperCase(), 
    publicInclusions + 
    makeClassDeclarations(moduleName, pgns));
}

function makeInfoComment(inputPath, outputPrefix) {
  return "/** Generated on " + Date() + " using \n *\n" + 
    " *  node codegen/index.js " + inputPath + " " + outputPrefix + "\n *\n */\n";

}

function outputData(outputPath, moduleName, interfaceData, implementationData, cb) {
  console.log(interfaceData);
  console.log(implementationData);

  interfaceFilename = Path.join(outputPath, moduleName + ".h");
  implementationFilename = Path.join(outputPath, moduleName + ".cpp");
  fs.writeFile(interfaceFilename, interfaceData, function(err) {
    if (err) {
      cb(err);
    } else {
      fs.writeFile(implementationFilename, implementationData, cb);
    }
  });
}

function compileXmlToCpp(value, inputPath, outputPath, cb) {
  var moduleName = "PgnClasses";
  try {
    var pgns = getPgnArrayFromParsedXml(value);
    var dup = getDuplicateId(pgns);
    assert(dup == undefined, "Ids are not unique: " + dup);
    cmt = makeInfoComment(inputPath, outputPath);
    var interfaceData = cmt + makeInterfaceFileContents(moduleName, pgns);
    var implementationData = cmt + makeImplementationFileContents(moduleName, pgns);
    outputData(outputPath, moduleName, interfaceData, implementationData, cb);
  } catch (e) {
    console.log('Caught exception while compiling C++');
    console.log(e);
    cb(e);
  }
}



function loadXml(inputPath, cb) {
  fs.readFile(inputPath, 'utf-8', function(err, data) {
    if (err) {
      cb(err);
    } else {
      parseString(data, cb);
    }
  });
}

function generate(inputPath, outputPrefix, cb) {
  loadXml(inputPath, function(err, value) {
    if (err) {
      cb(err);
    } else {
      compileXmlToCpp(value, inputPath, outputPrefix, cb);
    }
  });
}

module.exports.generate = generate;
