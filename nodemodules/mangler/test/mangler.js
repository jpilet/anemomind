var mangler = require('../index.js');
var eq = require('deep-equal-ident');
var assert = require('assert');

it('manger', function() {
    describe('Correctly mangle and demangle a datastructure', function() {
	var testbuf = new Buffer(2);
	testbuf[0] = 119;
	testbuf[1] = 255;
	
	var x = {
	    a: [1, 2, 3, [[{buf: testbuf}]]],
	    myStrings:
	    ["Some random string",
	     [[":I am a regular string who starts with a colon",
	       ":base64: I am a regular string who starts with :base64:"]]]
	};
	var y = mangler.demangle(mangler.mangle(x));
	var z = mangler.parse(mangler.stringify(x));
	assert(eq(x, y));
	assert(eq(x, z));
    });
});
