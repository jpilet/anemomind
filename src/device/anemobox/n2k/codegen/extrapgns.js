/*

Clarifications to request from B&G:

 To decode the data part, do we parse the 
 two bytes as a signed integer and multiply that 
 integer by 0.001 to obtain the performance ratio?

 So, suppose that we have the percentage 70%. Should it be encoded 
 as a signed integer 70%/0.1% = 0.7/0.001 = 700?

*/

var assert = require('assert');

function findField(fields, id) {
  return fields.Fields[0].Field.filter(function(f) {
    return f.Id == id;
  })[0];
}

// Try to generate the spec on the 
// same form as the XML.
function wrapFields(fields) {
  return [{"Field": fields}];
}

function wrapEnumPairs(pairs) {
  return [{
    "EnumPair": 
    pairs.map(function(p) {
      return {'$': p};
    })
  }];
}

function mergeInPlace(dst, added) {
  for (var k in added) {
    dst[k] = added[k];
  }
}

function bandgProperietarySingleFrame(
  topDecorations,
  fieldDecorations) {

  var dst = {
    "PGN": 65330,
    "Complete": true,
    "Priority": 3,
    "Frames": "Single",
    "Length": 8,
    "RepeatingFields": 0,

    // Currently we don't parse messages of this PGN,
    // and there is only one type of 65330 message.
    // But if we would start to parse, we should probably
    // look at all fields for which 'Match' is set to something,
    // and make a compound dispatch code based on all those fields.

    "Fields": wrapFields([{
        "Order": 1,
        "Id": "manufacturerId",
        "Name": "Manufacturer ID",
        "Description": "B&G",
        "BitLength": 16,
        "BitOffset": 0,
        "BitStart": 0,
        "Signed": false,
        "Match": 0x997d, // Right order? Yes! Unit tested.
        "Type": "Manufacturer code"
      }, {
        "Order": 3,
        "Id": "dataId",
        "Name": "Data ID",
        "BitLength": 12,
        "BitOffset": 16,
        "BitStart": 0,
        "Type": "Lookup table",
        "Signed": false,
        "EnumValues": wrapEnumPairs([{
          "Name": "VMG target percentage", 
          "Value": 285
        }])
      }, {
        "Order": 4,
        "Id": "length",
        "Name": "Data length",
        "BitLength": 4,
        "BitOffset": 28,
        "BitStart": 0,
        "Type": "Integer",
        "Signed": false,
        "Description": "Not sure if the length is in bytes or bits."
      }, {
        // This field depends on the dataId value.
        "Order": 5,
        "Id": "value",
        "Name": "Value",
        "BitOffset": 32,
        "BitStart": 0,
        "Signed": true,
        "BitLength": 32 // Assume all bits. But it depends on dataId.
      }])
  };
  mergeInPlace(dst, topDecorations);
  for (var k in fieldDecorations) {
    var f = findField(dst, k);
    assert(f);
    mergeInPlace(f, fieldDecorations[k]); 
  }
  return dst;
}

var all = [


/*
   The base message.

   Since we are currently only interested
   in outputting messages, no need to 
   implement full dispatch now.
   
   But if we would implement it, we would uncomment this code.


  bandgProperietarySingleFrame({
    "Id": "BandGVmgProprietaryPGN",
    "Description": "B&G Properietary PGN"
  }, {
  }, {
    "Description": "Depends on dataId",
  }),

*/


  // The specific message
  bandgProperietarySingleFrame({
    "Id": "BandGVmgPerformancePercentage",
    "Description": "B&G Properietary PGN -- VMG Performance %"
  }, {
    "dataId": {
      "Match": 285 // target VMG
    },
    "length": {
      "Match": 2, // Bytes or bits? It has to be bytes, because it is stored in 
                  // 4 bits meaning a max value of 15. And we want to store more
                  // than 15 bits, because the length is 2 bytes which is 16 bits.
                  // So the unit cannot be bits.
    },
    "value": {
      "BitLength": 16,
      "Signed": true,

      // Does it mean resolution 0.001?
      // We code things so that floating point value of 
      // 1.0 means 100 %.
      "Description": "'Each bit is 0.1%' according to NDA.", 

      "Resolution": 0.001
    }
  })
];

module.exports = all;
