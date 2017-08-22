var anemonode = require('../build/Release/anemonode');
var assert = require('assert');

function shouldNotWork(f) {
    var thrown = false;
    try {
	f();
    } catch (e) {
	thrown = true;
    }
    assert(thrown);
}


describe('anemonode', function() {
    it('log raw', function() {
	var logger = new anemonode.Logger();
	assert(logger);
	assert(logger.logRawNmea2000);
	
	var timestampMilliseconds = 123;
	var id = 99;
	var data = new Buffer("abc");

	logger.logRawNmea2000(timestampMilliseconds, id, data);

	shouldNotWork(function() {
	    logger.logRawNmea2000(timestampMilliseconds, id, data, data);
	});

	shouldNotWork(function() {
	    logger.logRawNmea2000(timestampMilliseconds, id, 9);
	});

	shouldNotWork(function() {
	    logger.logRawNmea2000(timestampMilliseconds, id, "asdfasdfasdf");
	});
    });
});
