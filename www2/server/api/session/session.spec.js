'use strict'

var should = require('should');
var app = require('../../app');
var request = require('supertest');
var Session = require('./session.model.js');
var mongoose = require('mongoose');
var Schema = mongoose.Schema;

var boatId = Schema.ObjectId(119);

function prepareRecord(cb) {
  Session.create({
    boat: boatId,
    _id: "s123",
    maxSpeedOverGround: 7.8,
    trajectoryLength: 8.9
  }, cb);
}

describe('Session', function() {
  it('GET /api/session', function(done) {
    prepareRecord(function(err, id) {
      if (err) {
        done(err);
      } else {
        request(app)
          .get('/api/session/s123')
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
      }
    });
  });
  
  it('GET /api/session/boat', function(done) {
    prepareRecord(function(err, id) {
      if (err) {
        done(err);
      } else {
        request(app)
          .get('/api/session/boat/119')
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
      }
    });    
  });
});
