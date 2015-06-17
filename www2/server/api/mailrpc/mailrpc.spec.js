'use strict';

var should = require('should');
var assert = require('assert');
var app = require('../../app');
var request = require('supertest');
var User = require('../user/user.model');
var Boat = require('../boat/boat.model');
var naming = require('mail/naming.js');
var common = require('mail/common.js');
var fs = require('fs');
var file = require('mail/file.js');
var coder = require('mail/json-coder.js');


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
  var remoteMailboxName = naming.makeMailboxNameFromBoatId(id);
  

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

  it('should fail to reset the mailbox', function(done) {
    server
      .get('/api/mailrpc/reset/' + remoteMailboxName)
      .set('Authorization', 'Bearer ' + token)
      .expect(403)
      .end(function(err, res) {
	if (err) return done(err);
	done();
      });
  });

  // Copy/pasted from boat.spec.js:
  // We need a boat in order to have a mailbox for that boat

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

  it('should successfully reset the mailbox', function(done) {
    server
      .get('/api/mailrpc/reset/' + remoteMailboxName)
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
	if (err) return done(err);
	done();
      });
  });
  
  it('should get the number of packets', function(done) {
    server
      .get('/api/mailrpc/getTotalPacketCount/' + remoteMailboxName)
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
	if (err) return done(err);
	JSON.parse(res.text).should.equal(0);
	done();
      });
  });

  it('should send a packet to another mailbox', function(done) {
    server
      .post('/api/mailrpc/sendPacket/' + remoteMailboxName)
      .send({dst: 'ccc', label: 0, data: new Buffer(4)})
      .set('Authorization', 'Bearer ' + token)
      .end(function(err, res) {
	if (err) return done(err);
	done();
      });
  });

  it('should get the number of packets', function(done) {
    server
      .get('/api/mailrpc/getTotalPacketCount/' + remoteMailboxName)
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
            [{src: "thebox", dst: remoteMailboxName,
              label: common.logfile,
              data: filedata, seqNumber: "2344"}]);
          server
            .post('/api/mailrpc/putPacket/' + remoteMailboxName)
            .send(postdata)
            .set('Authorization', 'Bearer ' + token)
            .expect(200)
            .end(function(err, res) {
              assert(!err);
              var uploadedFilename =
                "uploads/anemologs/boat123456789012345678901234/the_log_file.txt";
              fs.readFile(uploadedFilename, function(err, data0) {
                assert(!err);
                fs.readFile(p, function(err, data1) {
                  assert(!err);
                  assert(data0.equals(data1));
                  done();
                });
              });
            })
        });
      });
  });

  it('Should handle another incoming log file with the same name', function(done) {
    var p = '/tmp/the_log_file.txt';
    fs.writeFile(
      p, "Here there be boat logs.",
      function(err) {
        file.readAndPackFile(p, file.makeLogFileInfo(), function(err, filedata) {
          
          var postdata = coder.encodeArgs(
            [{packet: 'any'}],
            [{src: "thebox", dst: remoteMailboxName,
              label: common.logfile,
              data: filedata, seqNumber: "2345"}]);
          server
            .post('/api/mailrpc/putPacket/' + remoteMailboxName)
            .send(postdata)
            .set('Authorization', 'Bearer ' + token)
            .expect(200)
            .end(function(err, res) {
              assert(!err);
              var uploadedFilename =
                "uploads/anemologs/boat123456789012345678901234/the_log_file_version2.txt";
              fs.readFile(uploadedFilename, function(err, data0) {
                assert(err); // <-- We should not be able to load this file. Since we
                             //     the log file contains the same data, no new file
                             //     with be created.
                done();
              });
            })
        });
      });
  });

  it('Should handle another incoming log file with the same name, but different data',
       function(done) {
    var p = '/tmp/the_log_file.txt';
    fs.writeFile(
      p, "Here there be some other boat logs.",
      function(err) {
        file.readAndPackFile(p, file.makeLogFileInfo(), function(err, filedata) {
          var postdata = coder.encodeArgs(
            [{packet: 'any'}],
            [{src: "thebox", dst: remoteMailboxName,
              label: common.logfile,
              data: filedata, seqNumber: "2346"}]);
          server
            .post('/api/mailrpc/putPacket/' + remoteMailboxName)
            .send(postdata)
            .set('Authorization', 'Bearer ' + token)
            .expect(200)
            .end(function(err, res) {
              assert(!err);
              var uploadedFilename =
                "uploads/anemologs/boat123456789012345678901234/the_log_file_version2.txt";
              fs.readFile(uploadedFilename, function(err, data0) {
                assert(!err); 
                fs.readFile(p, function(err, data1) {
                  assert(!err);
                  assert(data0.equals(data1));
                  fs.unlink(
                    "uploads/anemologs/boat123456789012345678901234/the_log_file_version2.txt",
                    function(err) {
                      assert(!err);
                      fs.unlink(
                        "uploads/anemologs/boat123456789012345678901234/the_log_file.txt",
                        function(err) {
                          assert(!err);
                          done();
                        });
                    });
                });
              });
            })
        });
      });
  });

  after(function(done) {
    User.remove({email: "test@anemomind.com"}, done);
    Boat.remove({name: "TestBoat2"}).exec();
  });
});

