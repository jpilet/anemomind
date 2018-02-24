var im = require('immutable');
var assert = require('assert');

function hiddenFilename(x) {
  return typeof x == 'string' && 1 <= x.length && x[0] == '.';
}

function complement(f) {
  return function(x) {return !f(x);};
}

function endsWith(p, suffix) {
  return p.length >= suffix.length && p.slice(p.length - suffix.length) == suffix;
}

function asyncReduce(f, acc, src, cb) {
  if (src.isEmpty(src)) {
    cb(null, acc);
  } else {
    f(acc, src.first(), function(err, result) {
      if (err) {
        cb(err);
      } else {
        asyncReduce(f, result, src.rest(), cb);
      }
    });
  }
}

function asyncMap(f, src, cb) {
  return asyncReduce(function(acc, x, cb) {
    f(x, function(err, result) {
      if (err) {
        cb(err);
      } else {
        cb(null, acc.push(result));
      }
    });
  }, im.List(), src, cb);
} 

var STRIP_COMMENTS = /((\/\/.*$)|(\/\*[\s\S]*?\*\/))/mg;
var ARGUMENT_NAMES = /([^\s,]+)/g;
function getParamNames(func) {
  var fnStr = func.toString().replace(STRIP_COMMENTS, '');
  var result = fnStr.slice(fnStr.indexOf('(')+1, fnStr.indexOf(')')).match(ARGUMENT_NAMES);
  if(result === null)
     result = [];
  return result;
}

function isAsyncFunction(f) {
  var names = getParamNames(f);
  return 1 <= names.length && names[names.length-1] == "cb";
}

function toAsync(f) {
  if (isAsyncFunction(f)) {
    return f;
  } else {
    var names = getParamNames(f);
    assert(names.length == 1);
    return function(x, cb) {
      try {
        cb(null, f(x));
      } catch (e) {
        cb(e);
      }
    };
  }
}

function comp2(f, g) {
  var af = toAsync(f);
  var ag = toAsync(g);
  return function(x, cb) {
    ag(x, function(err, result) {
      if (err) {
        cb(err);
      } else {
        af(result, cb);
      }
    });
  };
}

function comp() {
  var arr = Array.prototype.slice.call(arguments);
  return arr.reduce(comp2);
}

function fwdcomp() {
  var arr = Array.prototype.slice.call(arguments);
  return arr.reverse().reduce(comp2);
}

function mergeDecorate(f) {
  var af = toAsync(f);
  return function(data, cb) {
    af(data, function(err, result) {
      if (err) {
        cb(err);
      } else {
        cb(null, data.merge(result));
      }
    });
  };
}

function decorateAt(key, f) {
  var af = toAsync(f);
  return function(data, cb) {
    af(data, function(err, result) {
      if (err) {
        cb(err);
      } else {
        cb(null, data.set(key, result));
      }
    });
  };
}

function forwardValue(x, cb) {
  return function(err) {
    cb(err, x);
  };
}

function dispatchOnKey(key, keyToFunctionMap) {
  return function(m, cb) {
    var value = m.get(key);
    var fun = keyToFunctionMap[value];
    return fun(m, cb);
  };
}

function removeKeys(keys) {
  return function(m) {
    return keys.reduce(function(m, k) {return m.delete(k);}, m);
  };
}

// There is a String method for this in newer stanards
function padStart(targetWidth, padChar, s) {
  var remaining = Math.max(0, targetWidth - s.length);
  return padChar.repeat(remaining) + s;
}


module.exports.padStart = padStart;
module.exports.removeKeys = removeKeys;
module.exports.forwardValue = forwardValue;
module.exports.mergeDecorate = mergeDecorate;
module.exports.decorateAt = decorateAt;
module.exports.endsWith = endsWith;
module.exports.getParamNames = getParamNames;
module.exports.hiddenFilename = hiddenFilename;
module.exports.complement = complement;
module.exports.asyncReduce = asyncReduce;
module.exports.asyncMap = asyncMap;
module.exports.isAsyncFunction = isAsyncFunction;
module.exports.comp2 = comp2;
module.exports.comp = comp;
module.exports.fwdcomp = fwdcomp;
module.exports.dispatchOnKey = dispatchOnKey;
