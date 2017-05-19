var exec = require('child_process').exec;
var assert = require('assert');

function segmentHeaderAndData(info) {
  // Sort by line count
  var lines = info.split('\n').map(function(x, i) {
    return {index: i, line: x};
  }).sort(function(a, b) {
    return a.line.length > b.line.length? -1 : 1
  });
  if (lines.length < 2) {
    return null;
  }

  // Pick the two longest lines...
  var headerAndData = lines.slice(0, 2).sort(function(a, b) {
    return a.index < b.index? -1 : 1;
  }).map(function(x) {return x.line;});
  return {
    // ...and return them
    header: headerAndData[0],
    data: headerAndData[1]
  };
}

function indexOf(full, sub) {
  var i = full.indexOf(sub);
  return i == -1? null : i;
}

// Break up a string into words (separated by space) and associate
// the start string position with every word.
function wordsWithPositions(s) {
  var words = s.split(' ');
  var at = 0;
  var dst = [];
  for (var i = 0; i < words.length; i++) {
    var word = words[i];
    dst.push({word: word, index: at});
    assert(word == s.slice(at, at + word.length));
    at += 1 + word.length;
  }
  return dst;
}

// Get the position at the middle of a word
function middle(x) {
  return x.index + 0.5*x.word.length;
}

// Pick the word whose middle is closest to 'at'
function findClosestValue(at, values) {
  function selectCloser(a, b) {
    return Math.abs(middle(a) - at) < Math.abs(middle(b) - at)?
      a : b;
  }
  return values.reduce(selectCloser);
}

// Try to convert string to node value, e.g. an integer.
function tryParse(x) {
  try {
    return eval(x);
  } catch (e) {
    return x;
  }
}

function tryApplyUnit(value, unit) {
  return (typeof value == "number" && typeof unit == "number")?
    value*unit : value;
}

// Parse the output of the "df -k" command
function parseDiskInfo(info) {
  var s = segmentHeaderAndData(info);
  if (s == null) {
    return null;
  }
  var headers = wordsWithPositions(s.header);
  var values = wordsWithPositions(s.data);
  var unit = 1024; // from the -k option
  function addValueItem(obj, s) {
    var toFind = s.find.toLowerCase();
    var h = headers.find(function(x) {return x.word.toLowerCase() == toFind;});
    if (h) {
      obj[s.key] = tryApplyUnit(
        tryParse(findClosestValue(middle(h), values).word), s.unit);
    }
    return obj;
  };
  return [{key: "available", find: "Available", unit:unit},
          {key: "available", find: "Avail", unit:unit},
          {key: "used", find: "Used", unit:unit},
          {key: "capacity", find: "Capacity"}].reduce(addValueItem, {});
}

function diskInfoAt(p, cb) {
  // See http://stackoverflow.com/a/8110535
  // https://www.mkssoftware.com/docs/man1/df.1.asp
  // -k Means the unit is 1024 bytes.
  exec('df -P -k ' + p, function(err, stdout, stderr) {
    if (err) {
      cb(err);
    } else {
      var info = parseDiskInfo(stdout);
      if (info) {
        cb(null, info);
      } else {
        cb(new Error("Failed to parse disk info from '%s'", stdout));
      }
    }
  });
}

module.exports.diskInfoAt = diskInfoAt;
module.exports.parseDiskInfo = parseDiskInfo;
