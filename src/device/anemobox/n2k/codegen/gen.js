var parseString = require('xml2js').parseString;
var fs = require('fs');
var assert = require('assert');
var Path = require('path');

function concat(a, b) {
  return a.concat(b);
}

function makeSet() {
  return {};
}

// http://stackoverflow.com/a/1026087
function capitalizeFirstLetter(string) {
    return string.charAt(0).toUpperCase() + string.slice(1);
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

// Indent lines stored in nested arrays
function indentLineArray(depth, lines) {
  if (lines instanceof Array) {
    return lines.map(function(x) {
      return indentLineArray(depth+1, x);
    }).join("");
  }
  return beginLine(depth-1) + lines;
}

function getDuplicate(pgns, f) {
  assert(f);
  var idSet = makeSet();
  for (var i = 0; i < pgns.length; i++) {
    id = f(pgns[i]);
    if (inSet(id, idSet)) {
      return id;
    }
    insertIntoSet(id, idSet);
  }
  return null;
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
  return xml.PGNDefinitions.PGNs[0].PGNInfo;
}

function getClassName(pgn) {
  return capitalizeFirstLetter(pgn.Id + '');
}

function getFieldArray(pgn) {
  if (pgn.Fields) {
    assert(pgn.Fields.length == 1);
    var fields = pgn.Fields[0];
    var arr = fields.Field;
    assert(arr instanceof Array);
    return arr;
  }
  return [];
}

function makeClassBlock(classComment, name, publicDecls, privateDecls, depth) {
  var indent = beginLine(depth);
  return '\n' + indent + "// " + classComment
    +indent + "class " + name + " {" 
    +indent + "public:"
    +publicDecls
    +indent + "private:"
    +privateDecls
    +indent + "};";
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

function getBitOffset(field) {
  return parseInt(field.BitOffset + '');
}


function makeIntegerReadExpr(field, srcName) {
  var signed = isSigned(field);
  var extractor = signed? "getSigned" : "getUnsigned";
  return srcName + "." + extractor + "(" + getBitLength(field) + ")";
}

function isData(field) {
  return getFieldId(field) == "data";
}

unitMap = {
  "m/s": {
    type: "sail::Velocity<double>",
    unit: "sail::Velocity<double>::metersPerSecond(1.0)",
  },
  "rad": {
    type: "sail::Angle<double>",
    unit: "sail::Angle<double>::radians(1.0)"
  },
  "deg": {
    type: "sail::Angle<double>",
    unit: "sail::Angle<double>::degrees(1.0)"
  },
  "m": {
    type: "sail::Length<double>",
    unit: "sail::Length<double>::meters(1.0)"
  },
  "s": {
    type: "sail::Duration<double>",
    unit: "sail::Duration<double>::seconds(1.0)"
  },
  "days": {
    type: "sail::Duration<double>",
    unit: "sail::Duration<double>::days(1.0)"
  },
  "minutes": {
    type: "sail::Duration<double>",
    unit: "sail::Duration<double>::minutes(1.0)"
  }
};

function getUnitInfo(field) {
  var unit = getUnits(field);
  assert(unit in unitMap, "Unit not recognized: " +  unit);
  return unitMap[unit];
}

function getFieldType(field) {
  if (isLookupTable(field)) {
    return getEnumClassName(field);
  } else if (isPhysicalQuantity(field)) {
    return getUnitInfo(field).type;
  } else if (isSigned(field)) {
    return "int64_t";
  } else if (isData(field)) {
    return "sail::Array<uint8_t>";
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
    + getAccessorName(field) + "() const {return "
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
    + makeAccessors(pgn, depth)
    + makeExtraMethodsInClass(pgn, depth);
}

function makeConstructorSignature(pgn) {
  return getClassName(pgn) + "(const uint8_t *data, int lengthBytes)";
}

function getFieldComment(field) {
  var d = field.Description;
  return (d? "// " + d : "");
}


function makeInstanceVariableDecls(pgn, depth) {
  var fields = getFieldArray(pgn);
  var commonDecls = beginLine(depth) + "bool _valid;";
  var s = commonDecls;
  for (var i = 0; i < fields.length; i++) {
    var field = fields[i];
    if (!skipField(field)) {
      s += beginLine(depth) 
        + getOptionalFieldType(field) + " "
        + getInstanceVariableName(field) + "; "
        + getFieldComment(field);
    }
  }
  return s;
}

function makeConstructorDecl(pgn, depth) {
  return beginLine(depth) + makeConstructorSignature(pgn) + ";";
}

function makePgnStaticConst(pgn, depth) {
  return beginLine(depth) + "static const int ThisPgn = " + getPgnCode(pgn) + ";";
}

function getType(field) {
  return field.Type + '';
}


function isLookupTable(field) {
  var t = getType(field);
  return t == "Lookup table" && field.EnumValues != null;
}

function getEnumClassName(field) {
  return capitalizeFirstLetter(getFieldId(field));
}

function removeUnnecessaryUnderscores(x) {
  return x.replace(/^_+/, "").replace(/_+/g,"_").replace(/_+$/,"");
}

function makeSymbolFromDescription(desc) {
  return removeUnnecessaryUnderscores(desc.replace(/\W/g, '_'));
}

function getEnumPairs(field) {
  return field.EnumValues[0].EnumPair.map(function(x) {
    for (var i in x) {
      var y = x[i];
      var value = parseInt(y.Value);
      var description = y.Name + '';
      return {
        value: value,
        description: description,
        symbol: makeSymbolFromDescription(description)
      };
    }
  });
}

function makeEnum(field, depth) {
  var outer = beginLine(depth);
  var inner = beginLine(depth+1);
  var pairs = getEnumPairs(field);
  return outer + "enum class " + getEnumClassName(field) + " {"
    + pairs.map(function(pair) {
      return inner + pair.symbol + " = " + pair.value;
    }).reduce(function(a, b) {return a + ", " + b;})
    +outer + "};";
}

function makeEnums(pgn, depth) {
  var enums = '';
  var fields = getFieldArray(pgn);
  for (var i = 0; i < fields.length; i++) {
    var field = fields[i];
    if (isLookupTable(field)) {
      enums += makeEnum(field, depth);
    }
  }
  return enums;
}

function makeClassDeclarationFromPgn(pgn, depth) {
  var innerDepth = depth + 1;
  return makeClassBlock(
    pgn.Description + '',
    getClassName(pgn), 
    makePgnStaticConst(pgn, innerDepth) 
      + makeEnums(pgn, innerDepth)
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

function getCommonPgnCode(pgnDefs) {
  assert(0 < pgnDefs.length);
  var code = getPgnCode(pgnDefs[0]);
  for (var i = 1; i < pgnDefs.length; i++) {
    assert(code == getPgnCode(pgnDefs[i]));
  }
  return code;
}

function concatReduce(x) {
  assert(x instanceof Array);
  if (x.length == 0) {
    return x;
  } else {
    return x.reduce(concat);
  }
}

function makePgnVariantDispatchers(multiDefs) {
  return concatReduce(concatReduce(multiDefs.map(function(pgnDefs) {
    var code = getCommonPgnCode(pgnDefs);
    var dispatchFields = getDispatchFields(pgnDefs);
    var tree = makeDispatchTree(pgnDefs, dispatchFields, makeIndexSet(pgnDefs.length));
    var other = listOtherDispatchFunctions(code, tree, []);
    var tname = makePgnEnumTypeName(code);
    return other.map(function(name) {
      return ["virtual " + tname + " " + name
              + "(const CanPacket &packet) {",
              ["return " + tname + "::Undefined; // TODO in derived class"],
              "}"];
    })
  })));
}

var canPacket = indentLineArray(1, ['\n', 'struct CanPacket {', [
  "std::string src;",
  "int pgn;", 
  "const uint8_t *data;", 
  "int length;"], "};"]);

function makeVisitorDeclaration(pgns) {
  var defMap = makeDefsPerPgn(pgns);
  var multiDefs = getMultiDefs(defMap);
  var s = [
    '\n\n',
    'class PgnVisitor {',
    ' public:',
    ['bool visit(const CanPacket &packet);',
     'virtual ~PgnVisitor() {}'],
    ' protected:',
    pgns.map(function(pgn) {
      return 'virtual bool apply'
        + '(const CanPacket& src, const ' + getClassName(pgn) + '& packet) { return false; }';
    }),
    makePgnVariantDispatchers(multiDefs),
    "};"
  ];
  return indentLineArray(1, s);
}

function makeDefsPerPgn(pgns) {
  var dst = {};
  for (var i = 0; i < pgns.length; i++) {
    var pgn = pgns[i];
    var n = getPgnCode(pgn);
    if (n in dst) {
      dst[n].push(pgn);
    } else {
      dst[n] = [pgn];
    }
  }
  return dst;
}

function getMultiDefs(defMap) {
  return Object.keys(defMap).map(function(key) {
    return defMap[key];
  }).filter(function(arr) {
    return 1 < arr.length;
  });
}

function makePgnEnumValueName(pgn) {
  return "Type" + getClassName(pgn);
}

function makePgnEnumTypeName(pgnCode) {
  return "PgnVariant" + pgnCode;
}

function getFieldMatch(field) {
  if (field.Match != null) {
    return parseInt(field.Match + '');
  }
  return null;
}


function getFieldsMarkedMatch(fields) {
  return fields.filter(function(f) {
    return f.Match != null;
  });
}


function addDispatchFieldEntries(defIndex, mf, occupied, dst) {
  for (var i = 0; i < mf.length; i++) {
    var field = mf[i];
    var offset = getBitOffset(field);
    var length = getBitLength(field);
    for (var i = 0; i < length; i++) {
      var index = offset + i;
      assert(!occupied[index], "Overlapping dispatch fields");
    }
    var e = dst[offset];
    var m = getFieldMatch(field);
    if(!dst[offset]) {
      dst[offset] = {
        offset: offset,
        length: length,
        lookup: {}
      };
    }
    e = dst[offset];
    assert(e.offset == offset, "Inconsistency");
    assert(e.length == length, "Inconsistency");
    if (!e.lookup[m]) {
      e.lookup[m] = makeSet();
    }
    insertIntoSet(defIndex, e.lookup[m]);
  }
  return dst;
}

function getSortedDispatchFields(m) {
  var arr = [];
  for (var key in m) {
    arr.push(m[key]);
  }
  arr.sort(function(a, b) {
    return a.offset < b.offset? -1 : 1;
  });
  return arr;
}

function getDispatchFields(pgnDefs) {
  var dst = {};
  var occupied = [];
  for (var i = 0; i < pgnDefs.length; i++) {
    var pgnDef = pgnDefs[i];
    var mf = getFieldsMarkedMatch(getFieldArray(pgnDef));
    addDispatchFieldEntries(i, mf, occupied, dst);
  }
  return getSortedDispatchFields(dst);
}

function makeDispatchVariableName(index) {
  return "dispatchCode" + index;
}

function makeDispatchCodeAssignments(pgnDefs, sortedFields) {
  var s = ["BitStream dispatchStream(packet.data, packet.length);"];
  var at = 0;
  for (var i = 0; i < sortedFields.length; i++) {
    var f = sortedFields[i];
    if (at < f.offset) {
      s.push("dispatchStream.advanceBits(" + f.offset - at + ");");
    }
    s.push("auto " + makeDispatchVariableName(i) + " = dispatchStream.getUnsigned("
           + f.length + ");");
    at = f.offset + f.length;
  }
  return s;
}

function intersect(setA, setB) {
  var dst = makeSet();
  for (var i in setA) {
    if (i in setB) {
      insertIntoSet(i, dst);
    }
  }
  return dst;
}


function makeIndexSet(n) {
  var s = makeSet();
  for (var i = 0; i < n; i++) {
    insertIntoSet(i, s);
  }
  return s;
}

function setToIntArray(src) {
  var dst = [];
  for (var i in src) {
    dst.push(parseInt(i));
  }
  return dst;
}

function makeDispatchTree(pgnDefs, sortedFields, subset) {
  if (sortedFields.length == 0) {
    return setToIntArray(subset).map(function(i) {
      return pgnDefs[i];
    });
  } else {
    var f = sortedFields[0];
    var dst = {};
    for (var key in f.lookup) {
      var lu = f.lookup[key];
      var subSubset = intersect(subset, lu);
      dst[key] = makeDispatchTree(pgnDefs, sortedFields.slice(1), subSubset);
    }
    return dst;
  }
}

function wrapCase(label, statements) {
  if (statements instanceof Array) {
    return [label + ": {", statements, ["break;"], "}"];
  } else {
    assert(typeof statements == "string");
    return [label  + ": " + statements];
  }
}

function makeSwitchStatement(map, switchExpression, defaultExpression, perCaseFunction) {
  return ["switch(" + switchExpression + ") {",
          Object.keys(map).map(function(key) {
            return wrapCase("case "+ key, perCaseFunction(key, map[key]));
          }).reduce(concat)
          .concat(wrapCase("default", defaultExpression)),
          "};"];
}


function makeVariantDispatchFunctionName(pgnCode, branches) {
  return "getDispatchCodeFor" + pgnCode + branches.map(function(x) {
    return "_" + x;
  }).join("");
}

function makePgnVariantMap(code, arr) {
  var dst = {};
  var t = makePgnEnumTypeName(code);
  for (var i = 0; i < arr.length; i++) {
    var pgn = arr[i];
    dst[t + "::" + makePgnEnumValueName(pgn)] = pgn;
  }
  return dst;
}

function makeOtherVariantDispatch(pgnCode, pgnDefs, branches) {
  assert(pgnDefs instanceof Array);
  return makeSwitchStatement(
    makePgnVariantMap(pgnCode, pgnDefs), 
    makeVariantDispatchFunctionName(pgnCode, branches) 
      + "(packet)",
    "return false;", function(key, pgnDef) {
      return callApplyMethod([pgnDef]);
    });
}

function makeVariantStatement(pgnCode, tree, branches) {
  if (tree instanceof Array) {
    if (tree.length == 1) {
      return callApplyMethod(tree);
    } else {
      return makeOtherVariantDispatch(pgnCode, tree, branches);
    }
  } else {
  return makeSwitchStatement(
    tree, makeDispatchVariableName(branches.length),
    "return false;",
    function(key, tree) {
      return makeVariantStatement(pgnCode, tree, branches.concat([key]));
    });
  }
}

function listOtherDispatchFunctions(pgnCode, tree, branches) {
  if (tree instanceof Array) {
    if (1 < tree.length) {
      return [makeVariantDispatchFunctionName(pgnCode, branches)];
    } else {
      return [];
    }
  } else {
    var result = [];
    for (var key in tree) {
      result = result.concat(listOtherDispatchFunctions(
        pgnCode, tree[key], branches.concat([key])));
    }
    return result;
  }
}

function callApplyMethod(pgnDefs) {
  if (pgnDefs.length == 1) {
    return 'return apply(packet, ' + getClassName(pgnDefs[0]) + '(packet.data, packet.length));';
  } else {
    var code = getCommonPgnCode(pgnDefs);
    var dispatchFields = getDispatchFields(pgnDefs);
    var tree = makeDispatchTree(pgnDefs, dispatchFields, makeIndexSet(pgnDefs.length));
    var assignments = makeDispatchCodeAssignments(pgnDefs, dispatchFields);
    var cases = makeVariantStatement(code, tree, []);
    return assignments.concat(cases);
  }
}




function makePgnEnum(pgnDefs) {
  if (pgnDefs.length == 1) {
    return null;
  } else {
    var code = getCommonPgnCode(pgnDefs);
    var symbols = pgnDefs.map(function(pgn) {
      return makePgnEnumValueName(pgn) + ",";
    });
    symbols.push("Undefined");
    return indentLineArray(
      1, ["enum class " + makePgnEnumTypeName(code) 
          + " {", symbols, "};\n"]);
  }
}

function makeVisitorImplementation(pgns) {
  return indentLineArray(0, [
    'bool PgnVisitor::visit(const CanPacket &packet) {',
    makeSwitchStatement(
      makeDefsPerPgn(pgns), "packet.pgn", "return false;", 
      function(key, pgnDefs) {
        return callApplyMethod(pgnDefs);
      }),
    ['return false;'],
    "}"]);
}


function makeInterface(label, pgns) {
  return wrapNamespace(
    label,
    makePgnEnums(pgns)
    + makeClassDeclarationsSub(pgns)
    + canPacket
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

function sortedIntegers(x) {
  for (var i = 0; i < x.length-1; i++) {
    if (x[i] > x[i+1]) {
      return false;
    }
  }
  return true;
}

function getEnumValueSet(field) {
  var intSet = getEnumPairs(field)
    .map(function(x) {
      return parseInt(x.value);
    });

  // Seems like the Javascript method
  // 'sort' converts elements to strings
  // before comparison. So integers are
  // not sorted the way you would expect!
  intSet.sort(function(a, b) {
    if (a < b) {
      return -1;
    } else if (a == b) {
      return 0;
    } else {
      return 1;
    }
  });
  assert(sortedIntegers(intSet));
  return "{" + intSet.join(", ") + "}";
}

function getEnumedFields(fields) {
  return fields.filter(function(field) {return isLookupTable(field)});
}

function allEnumedFieldsDefined(fields) {
  var enumed = getEnumedFields(fields);
  if (enumed.length == 0) {
    return "true";
  } else {
    return enumed.map(function(x) {
      return getInstanceVariableName(x) + ".defined()";
    }).join(" && ");
  }
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
    if (isPhysicalQuantity(field)) {
      var info = getUnitInfo(field);
      return lhs + "src.getPhysicalQuantity(" 
        + signedExpr + ", " + getResolution(field) 
        + ", " + info.unit + ", " + bits + ", " + offset + ");"
    } else if (isLookupTable(field)) {
      return lhs + "src.getUnsignedInSet(" 
        + bits + ", " + getEnumValueSet(field) + ").cast<" 
        + getFieldType(field) + ">();";
    } else if (isData(field)) {
      assert(bits % 8 == 0, 
             "Cannot read bytes, because the number of bits is not a multiple of 8.");
      return lhs + "src.readBytes(" + bits + ");"
    } else { // Something else.
      return lhs
        + (signed? "src.getSigned(" : "src.getUnsigned(")
        + bits + (signed? ", " + offset : "") 
        + ", N2kField::Definedness::AlwaysDefined);";
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
    + beginLine(innerDepth) + "_valid = " + allEnumedFieldsDefined(fields) + ";"
    + beginLine(depth) + "} else {"
    + beginLine(innerDepth) + "reset();"
    + beginLine(depth) + "}";
}


function makeConstructor(pgn, depth) {
  var innerDepth = depth + 1;
  return beginLine(depth, 1) + getClassName(pgn) + "::" + makeConstructorSignature(pgn) 
    + " {"
    + beginLine(innerDepth) + "N2kField::N2kFieldStream src(data, lengthBytes);"
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

function makeFieldMap(fields) {
  var dst = {};
  for (var i = 0; i < fields.length; i++) {
    var f = fields[i];
    dst[getFieldId(f)] = f;
  }
  return dst;
}

function getUniqueTimeStampName(fieldMap) {
  var candidate = "timeStamp";
  if (candidate in fieldMap) {
    return "anemomindTimeStamp";
  }
  return candidate;
}

function tryMakeTimeStampAccessor(fieldMap, depth) {
  if ("date" in fieldMap && "time" in fieldMap) {
    var date = fieldMap["date"];
    var time = fieldMap["time"];
    var name = getUniqueTimeStampName(fieldMap);
    return indentLineArray(
      depth, 
      ["sail::TimeStamp " + name + "() const {",
       ["return N2kField::getTimeStamp(*this);"],
       "}"]);
  }
  return "";
}

function makeExtraMethodsInClass(pgn, depth) {
  var fields = getFieldArray(pgn);
  var fieldMap = makeFieldMap(fields);
  return tryMakeTimeStampAccessor(fieldMap, depth);
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
    +'#include <device/anemobox/n2k/N2kField.h>\n'
    +'#include <server/common/Optional.h>\n\n';


function makePgnEnums(pgns) {
  var defMap = makeDefsPerPgn(pgns);
  return getMultiDefs(defMap).map(makePgnEnum).join("\n");
}

function makeInterfaceFileContents(moduleName, pgns) {
  return wrapInclusionGuard(
    moduleName.toUpperCase(), 
    publicInclusions + 
    makeInterface(moduleName, pgns));
}

function makeInfoComment(argv, inputPath) {
  return "/** Generated on " + Date() + " using \n *\n" + 
    " *     " + argv[0] + " " + argv[1] + " " + inputPath
    + "\n *"
    + "\n *  WARNING: Modifications to this file will be overwritten when it is re-generated"
    + "\n */\n";

}

function outputData(outputPath, moduleName, interfaceData, implementationData, 
                    summary, cb) {
  interfaceFilename = Path.join(outputPath, moduleName + ".h");
  implementationFilename = Path.join(outputPath, moduleName + ".cpp");
  summaryFilename = Path.join(outputPath, "summary.html");
  fs.writeFile(interfaceFilename, interfaceData, function(err) {
    if (err) {
      cb(err);
    } else {
      fs.writeFile(implementationFilename, implementationData, function(err) {
        if (err) {
          cb(err);
        } else {
          fs.writeFile(summaryFilename, summary, cb);
        }
      });
    }
  });
}

function getFieldSummary(field) {
  return getFieldId(field) + "(" + field.Name + ")";
}

function getFieldSummaries(fields) {
  return fields.map(getFieldSummary).join(", ");
}

function getPgnTableRow(pgn) {
  var summaries = getFieldSummaries(getFieldArray(pgn));
  return [pgn.PGN + '', pgn.Description + '', summaries];
}

function getPgnSummaries(pgns) {
  return pgns.map(getPgnTableRow);
}

function makeHtmlTable(nestedArrays) {
  return '<table>' + nestedArrays.map(function(arr) {
    return '<tr>' + arr.map(function(element) {
      return '<td>' + element + '</td>';
    }).join("") + '</tr>';
  }).join("") + '</table>';
}

var tableHeader = ["PGN", "Description", "Fields"];

function renderInPage(title, contents) {
  return '<html><head><title>' + title 
    + '</title><style>table, th, td { border: 1px solid black; }</style></head><body>'
    + contents + '</body></head>';
}

function makeSourceLink(src) {
  return '<p>Source <a href="' + src + '">' + src + '</a><p/>';
}

function getPgnId(pgn) {
  if (pgn.Id) {
    return pgn.Id + '';
  } else {
    console.log("PGN " + pgn.PGN + " has no Id");
    return null;
  }
}

function checkPgns(pgns) {
  var dup = getDuplicate(pgns, getPgnId);
  assert(dup == undefined, "PGN Ids are not unique: '" + dup + '"');
}


function compileAllFiles(argv, value, inputPath, outputPath, cb) {
  var moduleName = "PgnClasses";
  try {
    var allPgns = getPgnArrayFromParsedXml(value);
    var pgns = filterPgnsOfInterest(allPgns);
    
    checkPgns(pgns);
    var cmt = makeInfoComment(argv, inputPath);
    var interfaceData = cmt + makeInterfaceFileContents(moduleName, pgns);
    var implementationData = cmt + makeImplementationFileContents(moduleName, pgns);
    var summary = makeSourceLink(inputPath) + renderInPage(
      "PGN Summary", makeHtmlTable(
        [tableHeader].concat(getPgnSummaries(allPgns))));
    outputData(
      outputPath, moduleName, 
      interfaceData, implementationData, summary, cb);
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
      compileAllFiles(argv, value, inputPath, outputPath, cb);
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
