var traverse = require('traverse');

// This module does something similar to what
// json-buffer does. However, instead of providing
// stringify and parse, it returns JSON data structures
// with mangled buffers. The problem is that the default
// mangling of node buffers in JSON only works in one
// way: it is possible to map a buffer to a JSON string,
// but doing the inverse doesn't work. This module
// fixes that.
//
// Furthermore, it could be extended for other types
// than buffers as well, if needed.

function bufToString(x) {
    var b2 = new Buffer(x);
    //return ':base64:' + b2.toString('base64');
    return b2;
}

// HÄR: https://github.com/substack/js-traverse/issues/40

function mangle(data) {
  return traverse.map(
      data,
      function(x) {
	  console.log('x = %j', x);
	  console.log('identify the type now');
	  if (Buffer.isBuffer(x)) {
	      return bufToString(x);
	  } else if (typeof x == 'string') {
	      return (/^:/).test(x) ? ':colon:' + x : x;
	  }
	  return x;
      }
  );  
}

module.exports.mangle = mangle;

function demangle(x) {
    return traverse.map(
	data,
	function(x) {
	    return x;
	    
	    if (typeof x == 'string') {
		if((/^:base64:/).test(x)) {
		    return new Buffer(x.substring(8), 'base64');		    
		} else {
		    return (/^:colon:/).test(x) ? value.substring(7) : x;
		}
	    }
	    return x;
	}
    );
}

module.exports.demangle = demangle;



// These functions do the same as their JSON counterparts, except that they do mangling as well.
module.exports.stringify = function(x) {
    return JSON.stringify(mangle(x));
}

module.exports.parse = function(x) {
    return demangle(JSON.parse(x));
}




mangle(new Buffer(3));
