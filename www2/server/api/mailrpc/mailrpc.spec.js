'use strict';

var should = require('should');
var assert = require('assert');
var app = require('../../app');
var request = require('supertest');
var User = require('../user/user.model');


describe('/api/mailrpc', function() {
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
	server
	    .get('/api/mailrpc/reset/abc')
	    .set('Authorization', 'Bearer ' + token)
	    .expect(200)
	    .end(function(err, res) {
		if (err) return done(err);
		done();
	    });
    });
    
    it('should reset the mailbox', function(done) {
	server
	    .get('/api/mailrpc/getTotalPacketCount/abc')
	    .set('Authorization', 'Bearer ' + token)
	    .expect(200)
	    .end(function(err, res) {
		console.log('res.body = %j', res.body);
		for (var key in res) {
		    console.log('   %j = %j', key, res[key]);
		}
		if (err) return done(err);
		JSON.parse(res.text).should.equal(0);
		done();
	    });
    });


    // it('should return the mailbox name', function(done) {
    // 	callFunction(
    // 	    server,
    // 	    token,
    // 	    'getMailboxName',
    // 	    ['abc'],
    // 	    function(result) {
    // 		assert(result == 'abc');
    // 		done();
    // 	    }
    // 	);
    // });

    // it('should send a packet to another mailbox', function(done) {
    // 	callFunction(
    // 	    server,
    // 	    token,
    // 	    'sendPacket',
    // 	    ['abc', 'ccc', 0, new Buffer(4)],
    // 	    function(result) {
    // 		assert(result == undefined);
    // 		done();
    // 	    }
    // 	);
    // });

    // it('should get the number of packets, after the packet was sent', function(done) {
    // 	callFunction(
    // 	    server,
    // 	    token,
    // 	    'getTotalPacketCount',
    // 	    ['abc'],
    // 	    function (result) {
    // 		assert(result == 1);
    // 		done();
    // 	    }
    // 	);
    // });




  after(function(done) {
    User.remove({email: "test@anemomind.com"}, done);
  });
});

