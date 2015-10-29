'use strict'

var should = require('should');
var app = require('../../app');
var request = require('supertest');
var Summary = require('./summary.model.js');
var mongoose = require('mongoose');
var Schema = mongoose.Schema;

var boatId = Schema.ObjectId(119);

function prepareRecord(cb) {
  Summary.create({
    boat: boatId,
    curveId: "s123",
    maxSpeedOverGround: 7.8,
    trajectoryLength: 8.9
  }, cb);
}

describe('Summary', function() {
  it('GET /api/summary/session', function(done) {
    prepareRecord(function(err, id) {
      if (err) {
        done(err);
      } else {
        request(app)
          .get('/api/summary/session/s123')
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
  
  it('GET /api/summary/boat', function(done) {
    prepareRecord(function(err, id) {
      if (err) {
        done(err);
      } else {
        request(app)
          .get('/api/summary/boat/119')
          .expect(200)
          .end(function(err, res) {
            if (err) {
              done(err);
            } else {
              var body = res.body;
              res.body.should.be.instanceof(Array);
              var summary = res.body[0];
              summary.should.have.property('maxSpeedOverGround');
              summary.maxSpeedOverGround.should.equal(7.8);
              done();
            }
          });
      }
    });    
  });
});
