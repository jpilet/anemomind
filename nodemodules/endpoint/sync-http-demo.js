// A simulation of endpoint synchronizations
var mb = require("./mail.http.js");
var async = require("async");
var demo = require('./syncdemo-3.js');
var assert = require('assert');

//mongo --quiet anemomind-dev --eval "db.users.insert({name:'Test User','provider' : 'local', 'name' : 'test', 'email' : 'test@anemomind.com', 'hashedPassword' : 'bj0zHvlC/YIzEFOU7nKwr+OHEzSzfdFA9PMmsPGnWITGHp1zlL+29oa049o6FvuR2ofd8wOx2nBc5e2n2FIIsg==', 'salt' : 'bGwuseqg/L/do6vLH2sPVA==', 'role' : 'user' }"

var testuser = {
    'email': 'test@anemomind.com',
    'password': 'anemoTest'
};


var address = 'http://localhost:9000';

var boxnames = ["a", "b", "c"];


console.log('Try to synchronize three endpoints');
async.map(
    boxnames,
    function (boxname, cb) {
	mb.tryMakeEndpoint(
	    address, testuser, boxname,
	    function(err, endpoint) {
		assert.equal(err, undefined);

		// Always reset first so that we start from
		// the same state.
		endpoint.reset(function(err) {
		    cb(err, endpoint);
		});
	    }
	);
    },
    function(err, boxes) {
	assert.equal(err, undefined);
	demo.synchronizeThreeEndpoints(boxes, function(err) {
	    if (err) {
		console.log('THERE WAS AN ERROR: %j', err);
		console.log(
		    'Please make sure that there is a user like this on the server: %j',
		    testuser
		);
	    } else {
		console.log('SUCCESS!!!');
	    }
	});
    }
);


