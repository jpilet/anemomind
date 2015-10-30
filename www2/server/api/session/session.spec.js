'use strict'

var should = require('should');
var app = require('../../app');
var request = require('supertest');
var Session = require('./session.model.js');
var User = require('../user/user.model.js');
var Boat = require('../boat/boat.model.js');
var mongoose = require('mongoose');
var assert = require('assert');
var Schema = mongoose.Schema;

var boatId = Schema.ObjectId(119);

function prepareUser(cb) {
  User.remove().exec().then(function() {
    var testUser = new User({
      "provider" : "local",
      "name" : "test",
      "email" : "test@anemomind.com",
      "hashedPassword" : "bj0zHvlC/YIzEFOU7nKwr+OHEzSzfdFA9PMmsPGnWITGHp1zlL+29oa049o6FvuR2ofd8wOx2nBc5e2n2FIIsg==",
      "salt" : "bGwuseqg/L/do6vLH2sPVA==",
      "role" : "user"
    });
    testUser.save(cb);
  });
}

function getFirstUser(cb) {
  User.findOne({}, cb);
}

function getFirstBoat(cb) {
  Boat.findOne({}, cb);
}

var prepared = false;

function prepareBoat(cb) {
  assert(!prepared);
  prepared = true;
  getFirstUser(function(err, user) {
    Boat.remove().exec().then(function() {
      var testBoat = new Boat({
        name: "Frida",
        type: "IF",
        sailNumber: "1604",
        length: "8",
        lengthUnit: "meter",
        admins: [user._id],
        readers: [user._id]
      });
      testBoat.save(cb);
    });
  });
}

function prepareRecord(cb) {
  Session.remove({}, function(err) {
    if (err) {
      cb(err);
    } else {
      getFirstBoat(function(err, boat) {
        var boatId = boat._id;
        Session.create({
          boat: boatId,
          _id: "s123",
          maxSpeedOverGround: 7.8,
          trajectoryLength: 8.9
        }, function(err, sessionId) {
          cb(err, sessionId, boatId);
        });
      });
    }
  });
}


function prepareAll(cb) {
  prepareUser(function(err) {
    if (err) {
      cb(err);
    } else {
      prepareBoat(function(err) {
        if (err) {
          cb(err);
        } else {
          prepareRecord(cb);
        }
      });
    }
  });
}

describe('Session', function() {
  var server = request(app);
  var token;

  it('should give the test user an auth token', function(done) {
    prepareAll(function(err) {
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
  });
  
  it('GET /api/session', function(done) {
    server
      .get('/api/session/s123')
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
        if (err) {
          done(err);
        } else {
          res.body.should.have.property('maxSpeedOverGround');
          res.body.maxSpeedOverGround.should.equal(7.8);
          done();
        }
      });
  });
  
  it('GET /api/session/boat', function(done) {
    getFirstBoat(function(err, boat) {
      var boatId = boat._id;
      var addr = '/api/session/boat/' + boatId;
      server
        .get(addr)
        .set('Authorization', 'Bearer ' + token)
        .expect(200)
        .end(function(err, res) {
          if (err) {
            done(err);
          } else {
            var body = res.body;
            console.log(body);
            res.body.should.be.instanceof(Array);
            var session = res.body[0];
            session.should.have.property('maxSpeedOverGround');
            session.maxSpeedOverGround.should.equal(7.8);
            done();
          }
        });
    });
  });
  
});

