var anemonode = require('../build/Release/anemonode');
var assert = require('assert');

describe('anemonode', function() {
    it('log raw', function() {
	var logger = new anemonode.Logger();
	assert(logger);
	assert(logger.logRawNmea2000);
	
	var timestampMilliseconds = 123;
	var id = 99;
	var data = "abc";

	logger.logRawNmea2000(timestampMilliseconds, id, data);

	var thrown = false;
	try {
	    logger.logRawNmea2000(timestampMilliseconds, id, data, data);
	} catch (e) {
	    thrown = true;
	}

	assert(thrown);
    });
});
