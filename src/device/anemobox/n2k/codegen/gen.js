var parseString = require('xml2js').parseString;
var fs = require('fs');
var assert = require('assert');

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

function beginLine(depth) {
  return "\n" + makeWhitespace(depth);
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
  return beginLine(depth) + "class " + name + " {" 
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

unitMap = {
  "m/s": {
    type: "Velocity<double>",
    unit: "Velocity<double>::metersPerSecond(1.0)"
  }, 
  "rad": {
    type: "Angle<double>",
    unit: "Angle<double>::radians(1.0)"
  }
};

function getFieldType(field) {
  if (isPhysicalQuantity(field)) {
    var unit = getUnits(field);
    assert(unit in unitMap, "Unit not recognized: " +  unit);
    return unitMap[unit].type;
  } 
  return "int64_t";
}

function makeInstanceVariableDecls(pgn, depth) {
  fields = getFieldArray(pgn);
  var s = beginLine(depth) + "// Number of fields: " + fields.length;
  for (var i = 0; i < fields.length; i++) {
    var field = fields[i];
    s += beginLine(depth) 
      + getFieldType(field) + " "
      + getInstanceVariableName(field) + ";";
  }
  return s;
}

function makeClassDeclarationFromPgn(pgn, depth) {
  return makeClassBlock(
    getClassName(pgn), 
    "", 
    makeInstanceVariableDecls(pgn, depth+1), 
    depth);
  return getClassName(pgn);
}


function wrapNamespace(label, data) {
  return "namespace " + label + " {" + data + "\n}";
}

function makeClassDeclarationsSub(pgns) {
  depth = 1;
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

function makeInterfaceFileContents(moduleName, pgns) {
  return wrapInclusionGuard(moduleName.toUpperCase(), makeClassDeclarations(moduleName, pgns));
}

function makeInfoComment(inputPath, outputPrefix) {
  return "/** Generated on " + Date() + " using \n *\n" + 
    " *  node codegen/index.js " + inputPath + " " + outputPrefix + "\n *\n */\n";

}

function compileXmlToCpp(value, inputPath, outputPrefix, cb) {
  var moduleName = "PgnClasses";
  try {
    var pgns = getPgnArrayFromParsedXml(value);
    var dup = getDuplicateId(pgns);
    assert(dup == undefined, "Ids are not unique: " + dup);
    cmt = makeInfoComment(inputPath, outputPrefix);
    var interfaceData = cmt + makeInterfaceFileContents(moduleName, pgns);
    //var implementationData = cmt + makeImplementationFileContents(moduleName, pgns);
    console.log("Interface: ");
    console.log(interfaceData);
    cb(null, 'Success');
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
