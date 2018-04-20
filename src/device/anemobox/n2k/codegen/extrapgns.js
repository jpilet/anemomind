/*

Clarifications to request from B&G:

 To decode the data part, do we parse the 
 two bytes as a signed integer and multiply that 
 integer by 0.001 to obtain the performance ratio?

 So, suppose that we have the percentage 70%. Should it be encoded 
 as a signed integer 70%/0.1% = 0.7/0.001 = 700?

*/

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

var all = [{
  "PGN": 65330,
  "Complete": true,
  "Priority": 3,
  "Frames": "Single",
  "Length": 8,
  "RepeatingFields": 0,
  "Id": "BandGVmgPerformance",
  "Description": "B&G Proprietary PGN – VMG Performance",

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
    // This field is supposedly used for dispatching between
    // different types of B&G proprietary messages.
    "Order": 3,
    "Id": "dataId",
    "Name": "Data ID",
    "BitLength": 16,
    "BitOffset": 16,
    "BitStart": 0,
    "Type": "Lookup table",
    "Signed": false,
    "EnumValues": wrapEnumPairs([{
      "Name": "VMG target percentage", 
      "Value": 4637
    }, {
      "Name": "Course",
      "Value": 617
    }]),
  }, {
    // This field depends on the dataId value.
    "Order": 4,
    "condition": {"dataId": 4637},
    "Id": "vmgPerformance",
    "Name": "VMG Performance",
    "BitOffset": 32,
    "BitStart": 0,
    "BitLength": 16,
    "Signed": true,

    // Does it mean resolution 0.001?
    // We code things so that floating point value of 
    // 1.0 means 100 %.
    "Resolution": 0.001,
    "Description": "'Each bit is 0.1%' according to NDA. Regarding the decoded value, 1.0 means 100%"
  }, {
    // This field depends on the dataId value.
    "Order": 4,
    "condition": {"dataId": 617},
    "Id": "course",
    "Name": "Course",
    "BitOffset": 32,
    "BitStart": 0,
    "BitLength": 16,
    "Signed": false,
    "Units": "rad",
    "Resolution": 0.0001,
    "Description": "Each bit is 0.0001 radians, unsigned 2 bytes"
  }])
}];

module.exports = all;
