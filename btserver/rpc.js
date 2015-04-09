// Add all the functions that you
// want to make available over RPC here.
//
// Every function delivers its result by calling a callback.
// The return value should be undefined.
module.exports.testAdd = function(a, b, cb) {
    try {
	cb(undefined, a + b);
    } catch (e) {
	cb({
	    status: 'Failed to add numbers',
	    a: a,
	    b: b
	});
    }
}


