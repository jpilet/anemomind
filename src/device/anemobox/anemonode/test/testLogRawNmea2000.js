var anemonode = require('../build/Release/anemonode');
var assert = require('assert');

describe('anemonode', function() {
    it('log raw', function() {
	var logger = new anemonode.Logger();
	assert(logger);
	assert(logger.logRawNmea2000);
	
	var ts_sec = 123;
	var ts_usec = 1233;
	var id = 99;
	var data = "abc";

	logger.logRawNmea2000(ts_sec, ts_usec, id, data);

	var thrown = false;
	try {
	    logger.logRawNmea2000(ts_sec, ts_usec, id, ts_sec);
	} catch (e) {
	    thrown = true;
	}

	assert(thrown);
    });
});
