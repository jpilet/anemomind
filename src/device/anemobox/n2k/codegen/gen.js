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

function makeWhiteSpace(depth) {
  var s = '';
  for (var i = 0; i < depth; i++) {
    s += '  ';
  }
  return s;
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

function makeClassDeclarationFromPgn(pgn, indent) {
  makeClassBlock(
    capitalizeFirstLetter(pgn.Id),
    makePublicPgnDeclarations(pgn),
    makePrivatePgnDeclarations(pgn));
}

function compileXmlToCpp(value, outputPrefix, cb) {
  try {
    var pgns = getPgnArrayFromParsedXml(value);
    var dup = getDuplicateId(pgns);
    assert(dup == undefined, "Ids are not unique: " + dup);
    var interfaceData = makeInterfaceFileContents(pgns);
    var implementationData = makeImplementationFileContents(pgns);
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
      compileXmlToCpp(value, outputPrefix, cb);
    }
  });
}

module.exports.generate = generate;
