'use strict';

var should = require('should');
var app = require('../../app');
var request = require('supertest');
var User = require('../user/user.model');
var Boat = require('./boat.model');

describe('GET /api/boats', function() {
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

  it('should respond with an empty array', function(done) {
    request(app)
      .get('/api/boats')
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
        if (err) return done(err);
        res.body.should.be.instanceof(Array);
        res.body.should.have.length(0);
        done();
      });
  });

  it('should add a TestBoat', function(done) {
    request(app)
      .post('/api/boats')
      .set('Authorization', 'Bearer ' + token)
      .send({ name: 'TestBoat' })
      .expect(201)
      .end(function(err, res) {
        if (err) return done(err);
        done();
     });
   });

  it('should respond with an array containing TestBoat', function(done) {
    request(app)
      .get('/api/boats')
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
        if (err) return done(err);
        res.body.should.be.instanceof(Array);
        res.body.should.have.length(1);
        res.body[0].should.have.property('name');
        res.body[0].name.should.equal('TestBoat');
        done();
      });
  });

  after(function(done) {
    Boat.remove({name: "TestBoat"}).exec();
    User.remove({email: "test@anemomind.com"}, done);
  });
});

