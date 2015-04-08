'use strict';

var should = require('should');
var app = require('../../app');
var request = require('supertest');
var JSONB = require('json-buffer');
var User = require('../user/user.model');
//var Boat = require('../boat/boat.model');

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

    it('should get the number of packets', function(done) {
	server
	    .post('/api/mailrpc/getTotalPacketCount')
	    .set('Authorization', 'Bearer ' + token)
	    .send({args: ['abc']})
	    .expect(200)
	    .end(
		function(err, res) {
		    res.body.result.should.equal(JSONB.stringify(0));
		    done();
		}
	    );
    });


  after(function(done) {
    User.remove({email: "test@anemomind.com"}, done);
  });
});

