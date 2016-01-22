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

function indentLineArray(depth, lines) {
  return beginLine(depth) + lines.join(beginLine(depth));
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

function getPgnCode(x) {
  return parseInt(x.PGN);
}


function filterPgnsOfInterest(pgns) {
  return pgns.filter(function(x) {
    var pgn = getPgnCode(x);
    return inSet(pgn, pgnsOfInterest);
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
  var s = field["Signed"] + '';
  assert(s == "true" || s == "false");
  return s == "true";
}

function getBitLength(field) {
  return parseInt(field.BitLength + "");
}

function makeIntegerReadExpr(field, srcName) {
  var signed = isSigned(field);
  var extractor = signed? "getSigned" : "getUnsigned";
  return srcName + "." + extractor + "(" + getBitLength(field) + ")";
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

function isTypeKnown(field) {
  if (!isPhysicalQuantity(field)) { return true; }
  if (!(getUnits(field) in unitMap)) {
    console.log('Warning, unknown type: ' + getUnits(field));
    return false;
  }
  return true;
}

function wrapOptionalType(x) {
  return "Optional<" + x + " >";
}

function getOptionalFieldType(field) {
  return wrapOptionalType(getFieldType(field));
}

function getAccessorName(field) {
  return getFieldId(field);
}

function makeFieldAccessor(field) {
  return "const " + getOptionalFieldType(field) + " &" 
    + getAccessorName(field) + "() const {assert(_valid); return "
    + getInstanceVariableName(field) + ";}";
}

function makeAccessors(pgn, depth) {
  var fields = getFieldArray(pgn);
  var s = beginLine(depth, 1) + "// Field access";
  for (var i = 0; i < fields.length; i++) {
    var field = fields[i];
    if (!skipField(field)) {
      s += beginLine(depth) + makeFieldAccessor(field);
    }
  }
  return s;
}

var validMethod = "bool valid() const {return _valid;}";
var resetDecl = "void reset();";

var commonMethods = [
  validMethod,
  resetDecl
];


function makeCommonMethods(depth) {
  return commonMethods
    .map(function(x) {return beginLine(depth) + x;})
    .reduce(function(a, b) {
      return a + b
    });
}

function makeDefaultConstructorDecl(pgn, depth) {
  return beginLine(depth) + getClassName(pgn) + "();";
}

function makeResetMethod(pgn, depth) {
  var innerDepth = depth + 1;
  return beginLine(depth, 1) + "void " + getClassName(pgn) + "::reset() {"
    + beginLine(innerDepth) + "_valid = false;"
    + beginLine(depth) + "}";
}

function makeMethodsInClass(pgn, depth) {
  return "\n" + makeDefaultConstructorDecl(pgn, depth)
    + makeConstructorDecl(pgn, depth) 
    + makeCommonMethods(depth) 
    + makeAccessors(pgn, depth);
}

function makeConstructorSignature(pgn) {
  return getClassName(pgn) + "(const uint8_t *data, int lengthBytes)";
}

function makeInstanceVariableDecls(pgn, depth) {
  var fields = getFieldArray(pgn);
  var commonDecls = beginLine(depth) + "bool _valid;";
  var s = commonDecls + beginLine(depth) + "// Number of fields: " + fields.length;
  for (var i = 0; i < fields.length; i++) {
    var field = fields[i];
    if (!skipField(field)) {
      s += beginLine(depth) 
        + getOptionalFieldType(field) + " "
        + getInstanceVariableName(field) + ";";
    }
  }
  return s;
}

function makeConstructorDecl(pgn, depth) {
  return beginLine(depth) + makeConstructorSignature(pgn) + ";";
}

function makePgnStaticConst(pgn, depth) {
  return beginLine(depth) + "static const int pgn = " + getPgnCode(pgn) + ";";
}

function makeClassDeclarationFromPgn(pgn, depth) {
  var innerDepth = depth + 1;
  return makeClassBlock(
    getClassName(pgn), 
    makePgnStaticConst(pgn, innerDepth) 
      + makeMethodsInClass(pgn, innerDepth),
    makeInstanceVariableDecls(pgn, innerDepth), 
    depth);
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

function makeVisitorDeclaration(pgns) {
  var s = [
    '\n\nclass PgnVisitor {',
    ' public:',
    '  bool visit(int pgn, const uint8_t *data, int length);',
    '  virtual ~PgnVisitor() {}',
    ' protected:'
  ];
  for (var i = 0; i < pgns.length; i++) {
    s.push('  virtual bool apply'
           + '(const ' + getClassName(pgns[i]) + '& packet) { return false; }');
  }
  s.push('};\n');
  return s.join('\n');
}

function makeVisitorImplementation(pgns) {
  var s = [
    'bool PgnVisitor::visit(int pgn, const uint8_t *data, int length) {',
    '  switch(pgn) {'
  ];

  for (var i = 0; i < pgns.length; i++) {
    var pgn = pgns[i];
    s.push('    case ' + getPgnCode(pgn) + ': '
           + 'return apply(' + getClassName(pgn) + '(data, length));');
  }
  s.push('  }  // closes switch');
  s.push('  return false;');
  s.push('}\n');
  return s.join('\n');
}


function makeClassDeclarations(label, pgns) {
  return wrapNamespace(
    label,
    makeClassDeclarationsSub(pgns)
    + makeVisitorDeclaration(pgns));
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

function getResolution(field) {
  if (field.Resolution == null) {
    return "1.0";
  }
  return field.Resolution + '';
}

function getOffset(field) {
  if (field.Offset == null) {
    return "0";
  }
  return field.Offset + '';
}

function makeFieldFromIntExpr(field, intExpr) {
  var unit = getUnits(field);
  if (isPhysicalQuantity(field)) {
    var info = getUnitInfo(field);
    return "double(" 
      + (hasResolution(field)? field.Resolution + "*": "")
      + intExpr
      + ")*" + info.unit;
  }
  return intExpr;
}

function skipField(field) {
  if ((getFieldId(field) == 'reserved')
      || !isTypeKnown(field)) {
      return true;
  }
  return false;
}

function boolToString(x) {
  return x? "true" : "false";
}

function makeFieldAssignment(field, depth) {
  if (skipField(field)) {
    return indentLineArray(depth, [
      '// Skipping ' + getFieldId(field),
      'src.advanceBits(' + getBitLength(field) + ');'
    ]);
  } else {
    var lhs = beginLine(depth) + getInstanceVariableName(field) + " = ";
    var bits = getBitLength(field) + '';
    var signed = isSigned(field);
    var signedExpr = boolToString(signed);
    var offset = getOffset(field);
    console.log("signedExpr = " + typeof signedExpr);
    if (isPhysicalQuantity(field)) {
      var info = getUnitInfo(field);
      return lhs + "src.getPhysicalQuantity(" 
        + signedExpr + ", " + getResolution(field) 
        + ", " + info.unit + ", " + bits + ", " + offset + ");"
    } else { // Probably an enum. TODO: handle it more carefully here.
      return lhs
        + (signed? "src.getSigned(" : "src.getUnsigned(")
        + bits + (signed? ", " + offset : "") + ", false);";
    }
  }
}

function getTotalBitLength(fields) {
  var n = 0;
  for (var i = 0; i < fields.length; i++) {
    var field = fields[i];
    n += parseInt(field.BitLength);
  }
  return n;
}


function logIgnoringField(field, err) {
  console.log('Ignoring field ' + getFieldId(field) + ': ' + err);
}

function makeFieldAssignments(fields, depth) {
  var s = '';
  for (var i = 0; i < fields.length; i++) {
    s += makeFieldAssignment(fields[i], depth);
  }
  return s;
}


function makeConstructorStatements(pgn, depth) {
  var fields = getFieldArray(pgn);
  var innerDepth = depth + 1;
  return beginLine(depth) + 
    'if (' + getTotalBitLength(fields) + ' <= src.remainingBits()) {'
    + makeFieldAssignments(fields, innerDepth, false)
    + beginLine(innerDepth) + "_valid = true;"
    + beginLine(depth) + "} else {"
    + beginLine(innerDepth) + "reset();"
    + beginLine(depth) + "}";
}


function makeConstructor(pgn, depth) {
  var innerDepth = depth + 1;
  return beginLine(depth, 1) + getClassName(pgn) + "::" + makeConstructorSignature(pgn) 
    + " {"
    + beginLine(innerDepth) + "N2kField::Stream src(data, lengthBytes);"
    + makeConstructorStatements(pgn, innerDepth)
    + beginLine(depth) + "}";
}


function makeDefaultConstructor(pgn, depth) {
  var innerDepth = depth + 1;
  var fields = getFieldArray(pgn);
  var className = getClassName(pgn);
  return beginLine(depth, 1) + className + "::" + className + "() {"
    +beginLine(depth+1) + "reset();"
    +beginLine(depth) + "}";
}

function makeMethodsForPgn(pgn, depth) {
  return makeDefaultConstructor(pgn, depth) 
    + makeConstructor(pgn, depth)
    + makeResetMethod(pgn, depth);
}

var privateInclusions = '#include <device/anemobox/n2k/N2kField.h>\n\n'

function makeImplementationFileContents(moduleName, pgns) {
  var depth = 1;
  var contents = "";
  for (var i = 0; i < pgns.length; i++) {
    contents += makeMethodsForPgn(pgns[i], depth);
  }
  contents += '\n' + makeVisitorImplementation(pgns);
  return makeHeaderInclusion(moduleName) + privateInclusions + wrapNamespace(moduleName, contents);
}

var publicInclusions = '#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>\n'
    +'#include <cassert>\n'
    +'#include <server/common/Optional.h>\n\n';

function makeInterfaceFileContents(moduleName, pgns) {
  return wrapInclusionGuard(
    moduleName.toUpperCase(), 
    publicInclusions + 
    makeClassDeclarations(moduleName, pgns));
}

function makeInfoComment(argv, inputPath) {
  return "/** Generated on " + Date() + " using \n *\n" + 
    " *     " + argv[0] + " " + argv[1] + " " + inputPath
    + "\n *"
    + "\n *  WARNING: Modifications to this file will be overwritten when it is re-generated"
    + "\n */\n";

}

function outputData(outputPath, moduleName, interfaceData, implementationData, cb) {
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

function compileXmlToCpp(argv, value, inputPath, outputPath, cb) {
  var moduleName = "PgnClasses";
  try {
    var pgns = getPgnArrayFromParsedXml(value);
    var dup = getDuplicateId(pgns);
    assert(dup == undefined, "Ids are not unique: " + dup);
    var cmt = makeInfoComment(argv, inputPath);
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
      cb(new Error("Failed to read file " + inputPath));
    } else {
      parseString(data, cb);
    }
  });
}

function generate(argv, inputPath, outputPath, cb) {
  loadXml(inputPath, function(err, value) {
    if (err) {
      cb(err);
    } else {
      compileXmlToCpp(argv, value, inputPath, outputPath, cb);
    }
  });
}

defaultInputPath = '/home/jonas/programmering/cpp/canboat/analyzer/pgns.xml';

function getOutputPath(javascriptFilename) {
  dstLoc = "anemobox/n2k/"
  var index = javascriptFilename.indexOf(dstLoc);
  if (0 <= index) {
    return javascriptFilename.slice(0, index + dstLoc.length);
  }
  return null;
}

function main(argv) {
  inputPath = argv[2] || defaultInputPath;
  console.log("Input XML filename: " + inputPath);
  javascriptFilename = argv[1];
  outputPath = getOutputPath(javascriptFilename);
  if (outputPath == null) {
    console.log("Unable to determine output path from " + javascriptFilename);
  } else {
    console.log("Output generated files to " + outputPath);
    generate(argv, inputPath, outputPath, function(err, value) {
      if (err) {
        console.log("Failed to generate because ");
        console.log(err);
        console.log(err.stack);
      } else {
        console.log("Success!");
      }
    });
  }
}

module.exports.main = main;
