var c = require('../rpccodes.js');
var bigint = require('../bigint.js');
var assert = require('assert');
var eq = require('deep-equal-ident');

    

function wrapAndUnwrap(coder, x) {
    return coder.unwrap(coder.wrap(x));
}



it(
    'rpccodes',
    function() {
	describe(
	    'Should validate setForeignDiaryNumber',
	    function() {
		var args = {endpointName: bigint.make(12), diaryNumber: bigint.make(32)};
		var args2 = wrapAndUnwrap(c.setForeignDiaryNumber.args, args);
		assert(eq(args, args2));
	    }
	);

	describe(
	    'Should validate getFirstPacketStartingFrom',
	    function() {
		{
		    var args = {
			diaryNumber: bigint.make(13),
			lightWeight: true,
		    };
		    var args2 = wrapAndUnwrap(c.getFirstPacketStartingFrom.args, args);
		    assert(eq(args, args2));
		}{
		    var packet = {
			diaryNumber: bigint.make(39),
			src: bigint.make(12),
			dst: bigint.make(3),
			seqNumber: bigint.make(3999),
			cNumber: bigint.make(324111),
			label: 9,
			data: bigint.serialize('abcdef')
		    };
		    assert(eq(packet, wrapAndUnwrap(
			c.getFirstPacketStartingFrom.result, packet
		    )));
		}{
		    var packet = {
			diaryNumber: bigint.make(39),
			src: bigint.make(12),
			dst: bigint.make(3),
			seqNumber: bigint.make(3999),
		    };
		    assert(eq(packet, wrapAndUnwrap(
			c.getFirstPacketStartingFrom.result, packet
		    )));
		}
	    }
	);

	describe(
	    'Should validate handleIncomingPacket',
	    function() {
		var packet = {
		    diaryNumber: bigint.make(39),
		    src: bigint.make(12),
		    dst: bigint.make(3),
		    seqNumber: bigint.make(3999),
		    cNumber: bigint.make(324111),
		    label: 9,
		    data: bigint.serialize('abcdef')
		};
		assert(eq(packet, wrapAndUnwrap(c.handleIncomingPacket.args, packet)));
	    }
	);

	describe(
	    'isAdmissible',
	    function() {
		var args = {src: bigint.make(12), dst: bigint.make(144)};
		assert(eq(args, wrapAndUnwrap(c.isAdmissible.args, args)));
		var result = true;
		assert(eq(result, wrapAndUnwrap(c.isAdmissible.result, result)));
	    }
	);

	describe(
	    'getForeignStartNumber',
	    function() {
		var args = bigint.make(12335);
		assert(eq(args, wrapAndUnwrap(c.getForeignStartNumber.args,
						  args)));
		var result = bigint.make(99765);
		assert(eq(result, wrapAndUnwrap(c.getForeignStartNumber.result,
						    result)));
	    }
	);
    }
);
