'use strict';

var should = require('should');
var app = require('../../app');
var request = require('supertest');
var User = require('../user/user.model');
var Boat = require('./boat.model');
var testUtils = require('../testUtils.spec');
var Q = require('q');

describe('GET /api/boats', function() {

  var testUser;
  var readableBoat, writableBoat, publicBoat;

  before(function(done) {
    testUtils.addTestUser('test')
    .then(function(user) {
        testUser = user;
        return Q.all([
              testUtils.addTestBoat({name: "writable boat", admins: [ user.id ]}),
              testUtils.addTestBoat({name: "readable boat", readers: [ user.id ]}),
              testUtils.addTestBoat({name: "public boat", publicAccess: true})
        ])
      })
    .spread(function (boat1_, boat2_, publicBoat_) {
         writableBoat = boat1_;
         readableBoat = boat2_;
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

  it('should list 2 boats', function(done) {
    request(app)
      .get('/api/boats')
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
        if (err) return done(err);
        res.body.should.be.instanceof(Array);
        res.body.should.have.length(2);
        res.body[0].should.have.property('name');
        res.body[0].name.should.equal('writable boat');
        res.body[1].should.have.property('name');
        res.body[1].name.should.equal('readable boat');
        done();
      });
  });

  it('should respond with an array containing also the public boat', function(done) {
    request(app)
      .get('/api/boats?public=1')
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
        if (err) return done(err);
        res.body.should.be.instanceof(Array);
        res.body.should.have.length(3);
        res.body[2].should.have.property('name');
        res.body[2].name.should.equal('public boat');
        done();
      });
  });

  it('should respond with the details of the writable boat', function(done) {
    request(app)
      .get('/api/boats/' + writableBoat._id)
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
        if (err) return done(err);
        res.body.should.have.property('name');
        res.body.name.should.equal(writableBoat.name);
        res.body.should.have.property('_id');
        res.body._id.should.equal(writableBoat._id + '');
        done();
      });
  });

  it('should refuse to add a boat with an existing id', function(done) {
    request(app)
      .post('/api/boats')
      .set('Authorization', 'Bearer ' + token)
      .send({ _id: readableBoat._id, name: 'TestBoat3' })
      .expect(400)
      .end(done);
   });

  it('should list the public boat without auth', function(done) {
     request(app)
     .get('/api/boats?public=1')
     .expect(200)
     .end(function(err, res) {
        if (err) return done(err);
        res.body.should.be.instanceof(Array);
        res.body.should.have.length(1);
        res.body[0].should.have.property('name');
        res.body[0].name.should.equal('public boat');
        done();
      });
   });



  after(function(done) {
    testUtils.cleanup();
  });
});

