'use strict';

var should = require('should');
var app = require('../../app');
var request = require('supertest');

describe('GET /api/tiles/geojson', function() {

  it('should respond with a GeoJSON structure', function(done) {
    request(app)
      .get('/api/tiles/geojson/0/0/0/54f9a288a36444554e1173f0')
      .expect(200)
      .end(function(err, res) {
        if (err) return done(err);
        res.body.should.have.property("features");
        res.body.features.should.be.instanceof(Array).with.length(0);
        done();
      });
  });
});
