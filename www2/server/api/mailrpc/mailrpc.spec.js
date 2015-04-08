'use strict';

var should = require('should');
var assert = require('assert');
var app = require('../../app');
var request = require('supertest');
var JSONB = require('json-buffer');
var User = require('../user/user.model');

function callFunction(server, token, functionName, args, cb) {
    server
	.post('/api/mailrpc/' + functionName)
	.set('Authorization', 'Bearer ' + token)
	.send({args: JSONB.stringify(args)})
	.expect(200)
	.end(
	    function(err, res, body) {
		assert(err == undefined);
		cb(JSONB.parse(res.body.result));
	    }
	);
}

describe('POST /api/mailrpc', function() {
  before(function(done) {
    var testUser = new User({
      "provider" : "local",
      "name" : "test",
      "email" : "test@anemomind.com",
      "hashedPassword" : "bj0zHvlC/YIzEFOU7nKwr+OHEzSzfdFA9PMmsPGnWITGHp1zlL+29oa049o6FvuR2ofd8wOx2nBc5e2n2FIIsg==",
      "salt" : "bGwuseqg/L/do6vLH2sPVA==",
      "role" : "user"
    });
    testUser.save(done);
  });

    
  var server = request(app);
  var token;

  it('should give the test user an auth token', function(done) {
        server
            .post('/auth/local')
            .send({ email: 'test@anemomind.com', password: 'anemoTest' })
            .expect(200)
            .expect('Content-Type', /json/)
            .end(function (err, res) {
                 token = res.body.token;
                 if (err) return done(err);
                 return done();
             });
        });

    it('should reset the mailbox', function(done) {
	callFunction(
	    server,
	    token,
	    'reset',
	    ['abc'],
	    function (result) {
		assert(result == undefined);
		done();
	    }
	);
    });

    it('should get the number of packets', function(done) {
	callFunction(
	    server,
	    token,
	    'getTotalPacketCount',
	    ['abc'],
	    function (result) {
		assert(result == 0);
		done();
	    }
	);
    });

    it('should return the mailbox name', function(done) {
	callFunction(
	    server,
	    token,
	    'getMailboxName',
	    ['abc'],
	    function(result) {
		assert(result == 'abc');
		done();
	    }
	);
    });

    it('should send a packet to another mailbox', function(done) {
	callFunction(
	    server,
	    token,
	    'sendPacket',
	    ['abc', 'ccc', 0, new Buffer(4)],
	    function(result) {
		assert(result == undefined);
		done();
	    }
	);
    });

    it('should get the number of packets, after the packet was sent', function(done) {
	callFunction(
	    server,
	    token,
	    'getTotalPacketCount',
	    ['abc'],
	    function (result) {
		assert(result == 1);
		done();
	    }
	);
    });




  after(function(done) {
    User.remove({email: "test@anemomind.com"}, done);
  });
});

