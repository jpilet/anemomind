'use strict';

var should = require('should');
var request = require('supertest');
var rmdir = require('rimraf');

var config = require('../../config/environment');
var testUtils = require('../testUtils.spec');
var app = require('../../app');

describe('/api/files', function() {
  var userid;
  var boat;
  var server = request(app);

  before(function(done) {
    testUtils.addTestUser('test')
    .then(function(user) {
      return testUtils.addTestBoat({name: "test boat 1", admins: [ user.id ]});
    }).then(function(boat_) {
      boat = boat_;
    }).catch((err) => {
      console.warn(err);
    })
    .finally(done);
  });
      
  var token;

  it('should give the test user an auth token', function(done) {
        server
            .post('/auth/local')
            .send({ email: 'test@test.anemomind.com', password: 'anemoTest' })
            .expect(200)
            .expect('Content-Type', /json/)
            .end(function (err, res) {
              console.warn(res.body);
                 token = res.body.token;
                 if (err) return done(err);
                 return done();
             });
        });

  it('should accept a file upload', function(done) {
    request(app)
      .post('/api/files/' + boat.id)
      .set('Authorization', 'Bearer ' + token)
      .attach("file", __dirname + '/files.spec.js')
      .expect(201)
      .end(function(err, res) {
        if (err) return done(err);
        done();
      });
  });

  it('should accept 2 files upload', function(done) {
    const dataDir = __dirname + '/../../../../datasets/';
    request(app)
      .post('/api/files/' + boat.id)
      .set('Authorization', 'Bearer ' + token)
      .attach("file1", dataDir + 'tinylog.txt')
      .attach("file2", dataDir + 'nmea2000raw.log')
      .expect(201)
      .end(function(err, res) {
        if (err) return done(err);
        done();
      });
  });
  it('should list uploaded file', function(done) {
    request(app)
      .get('/api/files/' + boat.id)
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
        if (err) return done(err);
        res.body.should.have.length(3);
        for (let i = 0; i < 3; ++i) {
          res.body[i].should.have.property('name');
        }
        done();
      });
  });

  it('should delete an uploaded file', function(done) {
    request(app)
      .delete('/api/files/' + boat.id + '/files.spec.js')
      .set('Authorization', 'Bearer ' + token)
      .expect(204)
      .end(done);
  });


  after(function(done) {
    testUtils.cleanup();

    // cleanup the uploaded files
    rmdir(config.uploadDir + '/anemologs/boat' + boat.id, function() { done(); });
  });
});
