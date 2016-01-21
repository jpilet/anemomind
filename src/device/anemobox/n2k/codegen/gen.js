var parseString = require('xml2js').parseString;
var fs = require('fs');

function getPgnArrayFromParsedXml(xml) {
  return xml.PGNDefinitions.PGNs[0].PGNInfo;
}

function compileXmlToCpp(value, outputPrefix, cb) {
  var pgns = getPgnArrayFromParsedXml(value);
  cb(null, 'Success!');
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
