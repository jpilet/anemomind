var parseString = require('xml2js').parseString;
var fs = require('fs');
var assert = require('assert');
var Path = require('path');
var extra = require('./extrapgns.js');

function findValidPath(paths) {
  for (var i = 0; i < paths.length; i++) {
    var p = paths[i];
    if (fs.existsSync(p)) {
      return p;
    }
  }
  return null;
}

/**
   The pgn.Length value is probably not reliable in case 
   "repeating fields" is greater than 0. In that case,
   the length is effectiverly dependent on how many times the fields are
   repeated.

   Currently, the pgn.Length value is used to determine whether
   a packet should be parsed as a "fast packet" or an ordinary 
   packet. It seems like the pgn.Length is computed as the sum of bits
   for all fields and converted to bytes. See for instance GnssPositionData

*/

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
  var arr = xml.PGNDefinitions.PGNs[0].PGNInfo;
  for (var i = 0; i < extra.length; i++) {
    arr.push(extra[i]);
  }
  return arr;
}

function getClassName(pgn) {
  return capitalizeFirstLetter(pgn.Id + '');
}

function getFullFieldArraySub(pgn) {
  if (pgn.Fields) {
    assert(pgn.Fields.length == 1);
    var fields = pgn.Fields[0];
    var arr = fields.Field;
    assert(arr instanceof Array);
    return arr;
  }
  return [];
}

function arrayToMap(key, fields) {
  var m = {};
  for (var i = 0; i < fields.length; i++) {
    var f = fields[i];
    m[f[key]] = f;
  }
  return m;
}

function makeConditionForKey(field, expectedValue) {
  var variableName = getInstanceVariableName(field);
  return variableName + '.defined() && ' 
    + variableName + '.get() == static_cast<' + getFieldType(field) + '>(' 
    + expectedValue + ')';
}

function getFieldConditionExpression(fieldMap, field) {
  if (!('condition' in field)) {
    return null;
  }
  var exprs = [];
  var condition = field.condition;
  for (var k in condition) {
    var conditionField = fieldMap[k];
    var expectedValue = condition[k];

    // When decoding fields in the constructor, 
    // the value of the conditionField must be known
    // before we can evaluate the condition! This
    // is maybe not the simplest way, 
    // but probably the easiest way,
    // to ensure that is true.
    assert(conditionField.Order < field.Order);

    exprs.push(makeConditionForKey(conditionField, expectedValue));
  }
  return exprs.map(function(s) {return '(' + s + ')';}).join(' && ');
}

function decorateFields(fields) {
  var m = arrayToMap("Id", fields);
  for (var i = 0; i < fields.length; i++) {
    var f = fields[i];
    var expr = getFieldConditionExpression(m, f);
    if (expr) {
      f.conditionExpression = expr;
    }
  }
  return fields;
}

function getFullFieldArray(pgn) {
  return decorateFields(getFullFieldArraySub(pgn));
}


function getStaticFieldArray(pgn) {
  var r = pgn.RepeatingFields;
  var fields = getFullFieldArray(pgn);
  return r == 0? fields : fields.slice(0, -r);
}

function getRepeatingFieldArray(pgn) {
  var r = pgn.RepeatingFields;
  var fields = getFullFieldArray(pgn);
  var sliced = r == 0? [] : fields.slice(fields.length-r);
  assert(sliced.length == r);
  return sliced;
}

function complement(f) {
  return function(x) {return !f(x);}
}

function skipField(field) {
  if (getFieldId(field) == 'reserved') {
    return "Reserved field";
  } else if (getBitLength(field) == 0) {
    throw new Error("We don't know how to deal with this yet. Usually it means that the field is variable length.");
    return "Warning: Bit length 0 for field " + field.Name;
  }
  return false;
}

function getInstanceVariableFieldArray(pgn) {
  return getStaticFieldArray(pgn).filter(complement(skipField));
}

function makeClassBlock(classComment, name, publicDecls, privateDecls, depth) {
  return indentLineArray(depth, privateDecls? [
    "",
    "class " + name + " { // " + classCOmment,
    "public:",
    [publicDecls],
    "private:",
    [privateDecls],
    "};"
  ] : [
    "",
    "struct " + name + " { // " + classComment,
    [publicDecls],
    "};"
  ]);
}

function getFieldId(field) {
  return '' + field.Id;
}

function makeInstanceVariableName(x) {
  return x;
}

function getInstanceVariableName(field) {
  return makeInstanceVariableName(getFieldId(field));
}

function getLocalVariableName(field) {
  return 'l_' + field.Id;
}

function getUnits(field) {
  return '' + field.Units;
}

function isPhysicalQuantity(field) {
  return (field.Units != null && field.Units != ''
          && field.Units != 'hPa'); // hPa ignored for now
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

// NOT USED:
function makeIntegerReadExpr(field, srcName) {
  var signed = isSigned(field);
  var extractor = signed? "getSigned" : "getUnsigned";
  return srcName + "." + extractor + "(" + getBitLength(field) + ")";
}

function isData(field) {
  return getFieldId(field) == "data" || (getBitLength(field) > 64);
}

function isRational(field) {
  return hasResolution(field) 
    && !isPhysicalQuantity(field)
    && field.Type != "Integer";
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
  },
  "rad/s": {
    type: "sail::AngularVelocity<double>",
    unit: "(sail::Angle<double>::radians(1.0)/sail::Duration<double>::seconds(1.0))"
  },
  "rpm": {
    type: "sail::AngularVelocity<double>",
    unit: "(sail::Angle<double>::degrees(360)/sail::Duration<double>::minutes(1.0))"
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
  } else if (isRational(field)) {
    return "double";
  } else if (isSigned(field)) {
    return "int64_t";
  } else if (isData(field)) {
    return "sail::Array<uint8_t>";
  } else {
    return "uint64_t";
  }
}

function isTypeUnknown(field) {
  if (!isPhysicalQuantity(field)) { return false; }
  if (!(getUnits(field) in unitMap)) {
    var msg = 'Warning, unknown type: "' + getUnits(field) + '" of field named "' + field.Name + '"';
    console.log(msg);
    return msg;
  }
  return false;
}

function isTypeKnown(field) {
  return !isTypeUnknown(field);
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

var encodeDecl = "std::vector<uint8_t> encode() const override;";

var commonMethods = [
  "bool hasSomeData() const;",
  "bool hasAllData() const;",
  "bool valid() const;",
  encodeDecl
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

function makeMethodsInClass(pgn, depth) {
  return "\n" + makeDefaultConstructorDecl(pgn, depth)
    + makeConstructorDecl(pgn, depth) 
    + makeCommonMethods(depth) 
    + makeExtraMethodsInClass(pgn, depth);
}

function makeConstructorSignature(pgn) {
  return getClassName(pgn) + "(const uint8_t *data, int lengthBytes)";
}

function explainBits(bits0) {
  var bits = parseInt(bits0);
  var bytes = Math.floor(bits/8);
  var rem = (bits - 8*bytes);
  return bits + " bits = " + bytes + " bytes" 
    + (rem == 0? "" : (" + " + rem + " bits"));
}

function getFieldComment(field) {
  var d = field.Description;
  return "// " + (d? d : "") + " at " + explainBits(field.BitOffset);
}

function getMatchExpr(field) {
  var m = field.Match;
  if (m == undefined) {
    return null;
  }

  var pairs = getEnumPairs(field);
  if (pairs) {
    var e = pairs.filter(function(p) {return p.value == m;})[0];
    assert(e);
    return getEnumClassName(field) + "::" + e.symbol;
  } else {
    return m;
  }
}

function getFieldInitialization(field) {
  var e = getMatchExpr(field);
  return e == null? "" : (" = " + e);
}

function makeInstanceVariableDecl(field) {
  var skip = skipField(field);
  if (skip) {
    return "// Skip field '" + field.Name + "' of length " 
      + getBitLength(field) + " at " 
      + explainBits(field.BitOffset) + ": " + skip; 
  }
  return getOptionalFieldType(field) + " "
    + getInstanceVariableName(field)  
    + getFieldInitialization(field) + "; "
    + getFieldComment(field);
}


function makeInstanceVariableDecls(pgn, depth) {
  var fields = getStaticFieldArray(pgn);
  var commonDecls = beginLine(depth);
  var s = commonDecls;
  var bline = beginLine(depth);
  for (var i = 0; i < fields.length; i++) {
    var field = fields[i];
    s += bline + makeInstanceVariableDecl(field);
  }
  var rep = getRepeatingFieldArray(pgn);
  if (0 < rep.length) {
    s += bline + "std::vector<Repeating> repeating;";
  }
  return s;
}

function makeConstructorDecl(pgn, depth) {
  return beginLine(depth) + makeConstructorSignature(pgn) + ";";
}

function makePgnInfo(pgn, depth) {
  var code = getPgnCode(pgn);
  return indentLineArray(depth, [
    "static const int ThisPgn = " + code + ";",
    "int code() const override {return " + code + ";}"
  ]);
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
  var vals = field.EnumValues;
  if (!vals) {
    return null;
  }
  return vals[0].EnumPair.map(function(x) {
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
  var fields = getFullFieldArray(pgn);
  for (var i = 0; i < fields.length; i++) {
    var field = fields[i];
    if (isLookupTable(field)) {
      enums += makeEnum(field, depth);
    }
  }
  return enums;
}

function makeRepeatingFieldsStruct(pgn, depth) {
  var fields = getRepeatingFieldArray(pgn);
  if (getTotalBitLength(fields) == 0) {
    return '';
  }
  
  return indentLineArray(depth, [
    "struct Repeating {",
    fields.map(makeInstanceVariableDecl),
    "};"
  ]);
};

function makePgnClassInfo(pgn) {
  var rep = getRepeatingFieldArray(pgn);
  return "// Minimum size: " + explainBits(getTotalBitLength(getStaticFieldArray(pgn))) 
    + ". " + (rep.length == 0? "" : "Repeating struct size: " 
              + explainBits(getTotalBitLength(getRepeatingFieldArray(pgn))));
}

function makeClassDeclarationFromPgn(pgn, depth) {
  var innerDepth = depth + 1;
  return makeClassBlock(
    pgn.Description + '',
    getClassName(pgn) + ': public PgnBaseClass', 
    makePgnClassInfo(pgn)
      + makePgnInfo(pgn, innerDepth) 
      + makeEnums(pgn, innerDepth)
      + makeRepeatingFieldsStruct(pgn, innerDepth)
      + makeMethodsInClass(pgn, innerDepth)
      + makeInstanceVariableDecls(pgn, innerDepth), 
    null, depth);
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
              + "(const tN2kMsg &packet) {",
              ["return " + tname + "::Undefined; // TODO in derived class"],
              "}"];
    })
  })));
}

function makeVisitorDeclaration(pgns) {
  var defMap = makeDefsPerPgn(pgns);
  var multiDefs = getMultiDefs(defMap);
  var s = [
    '\n\n',
    'class PgnVisitor {',
    ' public:',
    [
      'bool visit(const tN2kMsg& packet);',
      '',
      '// You may have to split the packet, based on the pgn.',
     'virtual ~PgnVisitor() {}'],
    ' protected:',
    pgns.map(function(pgn) {
      return 'virtual bool apply'
        + '(const tN2kMsg& src, const ' + getClassName(pgn) + '& packet) { return false; }';
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
    var mf = getFieldsMarkedMatch(getStaticFieldArray(pgnDef));
    addDispatchFieldEntries(i, mf, occupied, dst);
  }
  return getSortedDispatchFields(dst);
}

function makeDispatchVariableName(index) {
  return "dispatchCode" + index;
}

function makeDispatchCodeAssignments(pgnDefs, sortedFields) {
  var s = ["BitStream dispatchStream(packet.Data, packet.DataLen);"];
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
    return 'return apply(packet, ' + getClassName(pgnDefs[0]) + '(packet.Data, packet.DataLen));';
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
    'bool PgnVisitor::visit(const tN2kMsg &packet) {',
    makeSwitchStatement(
      makeDefsPerPgn(pgns), "packet.PGN", "return false;", 
      function(key, pgnDefs) {
        return callApplyMethod(pgnDefs);
      }),
    ['return false;'],
    "}",
  ]);
}

/**

Actually, in the ttlappalainen NMEA2000 library, there is this function:

//*****************************************************************************
bool tNMEA2000::IsFastPacket(const tN2kMsg &N2kMsg) {
  if (N2kMsg.Priority>=0x80) return false; // Special handling for force to send message as single frame.
  
  return IsFastPacketPGN(N2kMsg.PGN);
}

It takes into account the special case of the "Priority" of a message being
greater than 0x80, which is something we currently ignore.

Apart from that, whether a packet is a fast packet or not is simply a function
of its PGN.

*/

function makeIsFastPacketImplementation(pgns) {
  var fastPacketPgns = pgns.filter(function(pgn) {
    return pgn.isFastPacket;
  });
  return indentLineArray(0, [
    "bool isFastPacket(int pgn) {",
    ["return (pgn == 129029); // TODO: This is just temporary."],
    "}"
  ]);
}


function makeInterface(label, pgns) {
  return wrapNamespace(
    label,
    pgnBaseClass
    + makePgnEnums(pgns)
    + makeClassDeclarationsSub(pgns)
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

function makeFieldAssignment(dstName, field) {
  if (skipField(field)) {
    return [
      '// Skipping ' + getFieldId(field),
      'src.advanceBits(' + getBitLength(field) + ');'
    ];
  } else {
    var lhs = dstName + " = ";
    var bits = getBitLength(field) + '';
    var signed = isSigned(field);
    var signedExpr = boolToString(signed);
    var offset = getOffset(field);
    var definedness = "N2kField::Definedness::" 
        + (8 < bits? "MaybeUndefined" : "AlwaysDefined");
    if (isPhysicalQuantity(field)) {
      var info = getUnitInfo(field);
      return lhs + "src.getPhysicalQuantity(" 
        + signedExpr + ", " + getResolution(field) 
        + ", " + info.unit + ", " + bits + ", " + offset + ");"
    } else if (isLookupTable(field)) {
      return lhs + "src.getUnsignedInSet(" 
        + bits + ", " + getEnumValueSet(field) + ").cast<" 
        + getFieldType(field) + ">();";
    } else if (isRational(field)){
      return lhs + "src.getDoubleWithResolution(" 
        + getResolution(field) + ", "
        + signedExpr + ", " + bits + ", " 
        + offset + ", " + definedness + ");";
    } else if (isData(field)) {
      assert(bits % 8 == 0, 
             "Cannot read bytes, because the number of bits is not a multiple of 8.");
      return lhs + "src.readBytes(" + bits + ");"
    } else { // Something else.
      return lhs
        + (signed? "src.getSigned(" : "src.getUnsigned(")
        + bits + (signed? ", " + offset : "") 
        + ", " + definedness + ");";
    }
  }
}

function add(a, b) {
  return a + b;
}

function makeEncodeFieldStatement(valueExpr, field) {
  var bits = getBitLength(field);
  if (skipField(field)) {
    return "dst.fillBits(" + bits + ", true); // TODO: "
      + "Can we safely do this? The field name is '" + field.Name + "'";
  } else {
    var signed = isSigned(field);
    var signedExpr = boolToString(signed);
    var offset = getOffset(field);
    if (isPhysicalQuantity(field)) {
      var info = getUnitInfo(field);
      return "dst.pushPhysicalQuantity(" 
        + signedExpr + ", " + getResolution(field) 
        + ", " + info.unit + ", " + bits + ", " + offset + ", " + valueExpr + ");";
    } else if (isLookupTable(field)) {
      return "dst.pushUnsigned(" 
        + bits + ", " + valueExpr + ".cast<uint64_t>());" 
    } else if (isRational(field)) {
      return "dst.pushDoubleWithResolution("
        + getResolution(field) + ", "
        + signedExpr + ", " + bits + ", " 
        + offset + ", " + valueExpr + ");";
    } else if (isData(field)) {
      assert(bits % 8 == 0, 
             "Cannot write bytes, because the number of bits is not a multiple of 8.");
      return "dst.pushBytes(" + bits + ", " + valueExpr + ");"
    } else { // Something else.
      return (signed? "dst.pushSigned(" : "dst.pushUnsigned(")
        + bits + (signed? ", " + offset : "") 
        + ", " + valueExpr + ");";
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

function conditionallyWrapCode(field, code) {
  var cond = field.conditionExpression;
  return cond? ['if (' + cond + ') {',
                [code],
                '}'] : code;
}

function makeConditionalFieldAssignment(dstName, f) {
  return conditionallyWrapCode(f, makeFieldAssignment(dstName, f));
}

function makeFieldAssignments(fields) {
  return fields.map(function(f) {
    return makeConditionalFieldAssignment(getInstanceVariableName(f), f);
  });
}

function bindRepeatingFieldToLocalVar(field) {
  return makeConditionalFieldAssignment("auto " + getLocalVariableName(field), field);
}
 
function bindRepeatingFieldsToLocalVars(fields) {
  return fields.map(bindRepeatingFieldToLocalVar);
}

function pushRepeatingFieldStruct(fields) {
  return [
    'repeating.push_back({' 
      + fields.filter(complement(skipField)).map(getLocalVariableName).join(", ") 
      + '});'];
}

function readRepeatingFields(fields) {
  var totalBits = getTotalBitLength(fields);
  if (totalBits == 0) {
    return '// No repeating fields.';
  }
  return [
    'while (' + totalBits + ' <= src.remainingBits()) {',
    bindRepeatingFieldsToLocalVars(fields),
    pushRepeatingFieldStruct(fields),
    '}'
  ]
}

function makeConstructorStatements(pgn, depth) {
  var fields = getStaticFieldArray(pgn);
  var innerDepth = depth + 1;

  // TODO! Proper handling of repeating fields!!!
  return indentLineArray(depth, [
    'if (' + getTotalBitLength(fields) + ' <= src.remainingBits()) {',
    makeFieldAssignments(fields),
    readRepeatingFields(getRepeatingFieldArray(pgn)),
    '}'
  ]);
}



function makeConstructor(pgn, depth) {
  var innerDepth = depth + 1;
  return beginLine(depth, 1) + getClassName(pgn) + "::" + makeConstructorSignature(pgn) 
    + " {"
    + beginLine(innerDepth) + "N2kField::N2kFieldStream src(data, lengthBytes);"
    + makeConstructorStatements(pgn, innerDepth)
    + beginLine(depth) + "}";
}

function hasData(pgn, op) {
  var fieldsDefined = getInstanceVariableFieldArray(pgn)
      .filter(function(f) {
        return !('condition' in f);
      })
      .map(function(f, i) {
    var s = getInstanceVariableName(f) + ".defined()";
    return (i == 0? "  " : op) + " " + s;
  });

  return ["return ", fieldsDefined, ";"];
}

function makeHasDataMethod(pgn, what, op, depth) {
  return indentLineArray(depth, [
    "bool " + getClassName(pgn) + "::has" + what + "Data() const {",
    hasData(pgn, op),
    "}"
  ]);
}

function makeConditionalEncodeFieldStatement(varName, field) {
  return conditionallyWrapCode(
    field, makeEncodeFieldStatement(varName, field));
}

function makeEncodeMethodStatements(pgn) {
  var dst = [
    "N2kField::N2kFieldOutputStream dst;",
    "if (!valid()) {", [
      'std::cerr << "Cannot encode ' + getClassName(pgn) + '";',
      "return {};"
    ],
    "}"
  ];
  
  var fields = getStaticFieldArray(pgn);

  // Currently, repeating fields are not dealt with.

  dst.push(fields.map(function(field) {
    var valueExpr = getInstanceVariableName(field);
    return makeConditionalEncodeFieldStatement(valueExpr, field);
  }));
  var repeating = getRepeatingFieldArray(pgn);
  if (0 < repeating.length) {
    dst.push('for (const auto& x: repeating) {');
    dst.push(repeating.filter(complement(skipField)).map(function(f) {
      return makeConditionalEncodeFieldStatement('x.' + getInstanceVariableName(f), f);
    }));
    dst.push('}');
  }
  dst.push("dst.fillUpToLength(8*8, true);");
  dst.push("return dst.moveData();");
  return dst;
}

function makeEncodeMethod(pgn, depth) {
  return indentLineArray(depth, [
    "std::vector<uint8_t> " + getClassName(pgn) + "::encode() const {",
    makeEncodeMethodStatements(pgn),
    "}"
  ]);
}


function makeDefaultConstructor(pgn, depth) {
  var innerDepth = depth + 1;
  var className = getClassName(pgn);
  return beginLine(depth, 1) + className + "::" + className + "() {"
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
  var fields = getStaticFieldArray(pgn);
  var fieldMap = makeFieldMap(fields);
  return tryMakeTimeStampAccessor(fieldMap, depth);
}

function matchesItsValue(f) {
  var fname = getInstanceVariableName(f);
  return " && " 
    + fname + ".defined() && " 
    + fname + ".get() == " + getMatchExpr(f);
}

function makeValidMethod(pgn) {
  var m = getStaticFieldArray(pgn).filter(function(f) {return "Match" in f;});
  return indentLineArray(1, [
    "bool " + getClassName(pgn) + "::valid() const {", [
      "return true",
      m.map(matchesItsValue),
      ";"
    ],
    "}"
  ]);
}

function makeMethodsForPgn(pgn, depth) {
  return makeDefaultConstructor(pgn, depth) 
    + makeConstructor(pgn, depth)
    + makeHasDataMethod(pgn, "Some", "||", depth)
    + makeHasDataMethod(pgn, "All", "&&", depth)
    + makeValidMethod(pgn)
    + makeEncodeMethod(pgn, depth);
}

var privateInclusions = '#include <device/anemobox/n2k/N2kField.h>\n#include<server/common/logging.h>\n#include <iostream>\n\n';

function makeImplementationFileContents(moduleName, pgns) {
  var depth = 1;
  var contents = "";
  for (var i = 0; i < pgns.length; i++) {
    contents += makeMethodsForPgn(pgns[i], depth);
  }
  contents += '\n' + makeIsFastPacketImplementation(pgns);
  contents += '\n' + makeVisitorImplementation(pgns);
  return makeHeaderInclusion(moduleName) + privateInclusions + wrapNamespace(moduleName, contents);
}

var publicInclusions = '#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>\n'
    +'#include <cassert>\n'
    +'#include <device/anemobox/n2k/N2kField.h>\n'
    +'#include <server/common/Optional.h>\n'
  +'#include <N2kMsg.h>\n\n';


function makePgnEnums(pgns) {
  var defMap = makeDefsPerPgn(pgns);
  return getMultiDefs(defMap).map(makePgnEnum).join("\n");
}

var pgnBaseClass = indentLineArray(1, [
  'class PgnBaseClass {',
  'public:', [
    'virtual int code() const = 0;',
    'virtual std::vector<uint8_t> encode() const = 0;',
    'virtual ~PgnBaseClass() {}'
  ],
  '};',
  ''
]);

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

function writeFiles(filenameTextPairs, cb) {
  if (filenameTextPairs.length == 0) {
    cb();
  } else {
    var first = filenameTextPairs[0];
    fs.writeFile(first[0], first[1], 'utf8', function(err) {
      if (err) {
        cb(err);
      } else {
        var rest = filenameTextPairs.slice(1);
        writeFiles(rest, cb);
      }
    });
  }
}


function getFieldSummary(field) {
  return getFieldId(field) + "(" + field.Name + ")";
}

function getFieldSummaries(fields) {
  return fields.map(getFieldSummary).join(", ");
}

function getPgnTableRow(pgn) {
  var summaries = getFieldSummaries(getFullFieldArray(pgn));
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

function compileAllCppFiles(argv, pgns, inputPath, outputPath) {
  var moduleName = "PgnClasses";
  checkPgns(pgns);
  var cmt = makeInfoComment(argv, inputPath);
  var interfaceData = cmt + makeInterfaceFileContents(moduleName, pgns);
  var implementationData = cmt + makeImplementationFileContents(moduleName, pgns);
  var interfaceName = Path.join(outputPath, moduleName + ".h");
  var implementationName = Path.join(outputPath, moduleName + ".cpp");
  return [
    [interfaceName, interfaceData], 
    [implementationName, implementationData]
  ];
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

function generatePgnTableJs(pgns) {
  return indentLineArray(0, [
    "module.exports = {",
    pgns.map(function(pgn) {
      return pgn.Id + ": " + getPgnCode(pgn) + ",";
    }),
    "};"
  ]);
}

function generate(argv, inputPath, outputPath, cb) {
  loadXml(inputPath, function(err, value) {
    if (err) {
      cb(err);
    } else {
      try {
        var allPgns = getPgnArrayFromParsedXml(value);
        var pgns = filterPgnsOfInterest(allPgns);

        // A html table
        var summaryFilename = outputPath + "/summary.html";
        var summary = makeSourceLink(inputPath) + renderInPage(
          "PGN Summary", makeHtmlTable(
            [tableHeader].concat(getPgnSummaries(allPgns))));

        // A node module, convenient for getting the right code.
        var jsTableFilename = outputPath 
            + "/../anemonode/components/pgntable.js";
        var jsTable = generatePgnTableJs(pgns);

        // An array of (filename,data) pairs
        var allData = 
            compileAllCppFiles(argv, pgns, inputPath, outputPath)
            .concat([
              [summaryFilename, summary],
              [jsTableFilename, jsTable]
            ]);

        writeFiles(allData, cb);
      } catch (e) {
        cb(e);
      }
    }
  });
}

var inputPathsToTry = [
  '/Users/leto/Documents/anemomind/canboat/analyzer/pgns.xml',
  '/Users/jonas/prog/canboat/analyzer/pgns.xml'
];

function getOutputPath(javascriptFilename) {
  dstLoc = "anemobox/n2k/"
  var index = javascriptFilename.indexOf(dstLoc);
  if (0 <= index) {
    return javascriptFilename.slice(0, index + dstLoc.length);
  }
  return null;
}

function main(argv) {
  var paths = [argv[2]].concat(inputPathsToTry);
  var inputPath = findValidPath(paths);
  console.log("Input XML filename: " + inputPath);
  javascriptFilename = argv[1];
  outputPath = getOutputPath(javascriptFilename);
  if (outputPath == null) {
    console.log("Unable to determine output path from " + javascriptFilename);
  } else {
    generate(argv, inputPath, outputPath, function(err, value) {
      if (err) {
        console.log("Failed to generate because ");
        console.log(err);
        console.log(err.stack);
      } else {
        console.log('Output path: ' + outputPath);
        console.log("Success!");
      }
    });
  }
}

module.exports.main = main;
