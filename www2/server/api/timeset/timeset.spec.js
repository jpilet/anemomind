'use strict'

var should = require('should');
var app = require('../../app');
var request = require('supertest');
var Timeset = require('./timeset.model.js');
var User = require('../user/user.model.js');
var Boat = require('../boat/boat.model.js');
var mongoose = require('mongoose');
var assert = require('assert');
var Schema = mongoose.Schema;
var utils = require('../testUtils.spec.js');

var d = new Date();

function prepareRecord(boat, cb) {
  Timeset.remove({}, function(err) {
    if (err) {
      cb(err);
    } else {
      var boatId = boat._id;
      var ts = new Timeset({
        boat: boatId,
        type: "delete",
        begin: d,
        end: d + 2000
      });
      ts.save(function(err, timesetId) {
        cb(err, timesetId, boatId);
      });
      cb(err, ts);
    }
  });
}


function prepareAll(cb) {
  utils.addTestUser().then(function(user) {
    utils.addTestBoat().then(function(boat) {
      prepareRecord(boat, function(err, data) {
        cb(err, user, boat, data);
      });
    });
  });
}

describe('////////////////// Timeset', function() {
  var server = request(app);
  var token;
  var user;
  var boat;
  var data;


  it('should give the test user an auth token', function(done) {
    prepareAll(function(err, u, b, d) {
      user = u;
      data = d;
      boat = b;
      console.log(' err: %j', err);
      console.log('user: %j', u);
      console.log('boat: %j', b);
      console.log('data: %j', d);
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

  it('GET /api/timeset', function(done) {
    server
      .get('/api/timeset/' + boat._id)
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
        if (err) {
          done(err);
        } else {
          done();
        }
      });
  });
  /*
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
            res.body.should.be.instanceof(Array);
            var session = res.body[0];
            session.should.have.property('maxSpeedOverGround');
            session.maxSpeedOverGround.should.equal(7.8);
            done();
          }
        });
    });
  });*/
  
});

