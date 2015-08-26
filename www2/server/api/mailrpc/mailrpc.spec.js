'use strict';
var should = require('should');
var assert = require('assert');
var app = require('../../app');
var request = require('supertest');
var User = require('../user/user.model');
var Boat = require('../boat/boat.model');
var naming = require('endpoint/naming.js');
var common = require('endpoint/common.js');
var fs = require('fs');
var file = require('endpoint/logfile.js');
var coder = require('endpoint/json-coder.js');


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
  
  var id ="123456789012345678901234";
  var remoteEndpointName = naming.makeEndpointNameFromBoatId(id);
  

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

  it('should fail to reset the endpoint', function(done) {
    server
      .get('/api/mailrpc/reset/' + remoteEndpointName)
      .set('Authorization', 'Bearer ' + token)
      .expect(403)
      .end(function(err, res) {
	if (err) return done(err);
	done();
      });
  });

  // Copy/pasted from boat.spec.js:
  // We need a boat in order to have a endpoint for that boat

  it('should add a TestBoat2 with a given _id', function(done) {
    request(app)
      .post('/api/boats')
      .set('Authorization', 'Bearer ' + token)
      .send({ _id: id, name: 'TestBoat2' })
      .expect(201)
      .end(function(err, res) {
	if (err) return done(err);
	res.body.should.have.property('_id');
	res.body._id.should.equal(id);
	done();
      });
  });

  it('should successfully reset the endpoint', function(done) {
    server
      .get('/api/mailrpc/reset/' + remoteEndpointName)
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
	if (err) return done(err);
	done();
      });
  });
  
  it('should get the number of packets', function(done) {
    server
      .get('/api/mailrpc/getTotalPacketCount/' + remoteEndpointName)
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
	if (err) return done(err);
	JSON.parse(res.text).should.equal(0);
	done();
      });
  });

  it('should send a packet to another endpoint', function(done) {
    server
      .post('/api/mailrpc/sendPacket/' + remoteEndpointName)
      .send({dst: 'ccc', label: 0, data: new Buffer(4)})
      .set('Authorization', 'Bearer ' + token)
      .end(function(err, res) {
	if (err) return done(err);
	done();
      });
  });

  it('should get the number of packets', function(done) {
    server
      .get('/api/mailrpc/getTotalPacketCount/' + remoteEndpointName)
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
	if (err) return done(err);
	JSON.parse(res.text).should.equal(1);
	done();
      });
  });

  it('Should handle an incoming log file', function(done) {
    var p = '/tmp/the_log_file.txt';
    fs.writeFile(
      p, "Here there be boat logs.",
      function(err) {
        file.readAndPackFile(p, file.makeLogFileInfo(), function(err, filedata) {
          
          var postdata = coder.encodeArgs(
            [{packet: 'any'}],
            [{src: "thebox", dst: remoteEndpointName,
              label: common.logfile,
              data: filedata, seqNumber: "0000014e1ad6b2b2"}]);
          server
            .post('/api/mailrpc/putPacket/' + remoteEndpointName)
            .send(postdata)
            .set('Authorization', 'Bearer ' + token)
            .expect(200)
            .end(function(err, res) {
              assert(!err);
              done();
            });
        })
      });
  });


  it('Should read the updated number', function(done) {
    var lbData = coder.encodeArgs(
      [{pairs: 'any'}],
      [[{src: 'thebox', dst: remoteEndpointName}]]
    );
    console.log('lbData:');
    console.log(lbData);
    server.post('/api/mailrpc/updateLowerBounds/' + remoteEndpointName)
      .send(lbData)
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
        assert(res.body[0] == "0000014e1ad6b2b3");
        assert(!err);
        done();
      });
  });


  after(function(done) {
    User.remove({email: "test@anemomind.com"}, done);
    Boat.remove({name: "TestBoat2"}).exec();
  });
});

