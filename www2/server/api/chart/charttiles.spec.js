'use strict';

var should = require('should');
var app = require('../../app');
var request = require('supertest');
var testUtils = require('../testUtils.spec');
var Q = require('q');

describe('GET /api/chart/', function() {
  var boat1, publicBoat;

  before(function(done) {
    testUtils.addTestUser('test')
    .then(function(user) {
      return Q.all([
        testUtils.addTestBoat({name: "test boat 1", admins: [ user.id ]}),
        testUtils.addTestBoat({name: "test boat public", publicAccess: true})
        ])
      })
    .spread(function (boat1_, publicBoat_) {
            boat1 = boat1_;
            publicBoat = publicBoat_;
            done();
    })
    .catch(function (err) {
      console.warn(err);
      done(err);
    });
  });

  var server = request(app);
  var token;

  it('should give the test user an auth token', function(done) {
        server
            .post('/auth/local')
            .send({ email: 'test@test.anemomind.com', password: 'anemoTest' })
            .expect(200)
            .expect('Content-Type', /json/)
            .end(function (err, res) {
                 token = res.body.token;
                 if (err) return done(err);
                 return done();
             });
        });

  it('should respond with a GeoJSON structure', function(done) {
    server
      .get('/api/tiles/geojson/0/0/0/' + boat1._id)
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
        if (err) return done(err);
        res.body.should.have.property("features");
        res.body.features.should.be.instanceof(Array).with.length(0);
        done();
      });
  });

  it('should serve a tile from a public boat without auth', function(done) {
    server
      .get('/api/tiles/geojson/0/0/0/' + publicBoat._id)
      .expect(200)
      .end(function(err, res) {
        if (err) return done(err);
        res.body.should.have.property("features");
        res.body.features.should.be.instanceof(Array).with.length(0);
        done();
      });
  });

  it('should refuse to serve without auth', function(done) {
    server
      .get('/api/tiles/geojson/0/0/0/' + boat1._id)
      .expect(401)
      .end(done);
  });

  after(function(done) {
    testUtils.cleanup();
    done();
  });
});
