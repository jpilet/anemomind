'use strict';

var should = require('should');
var app = require('../../app');
var request = require('supertest');
var User = require('../user/user.model');
var Boat = require('../boat/boat.model');
var Event = require('./event.model');
var Q = require('q');
var rmdir = require('rimraf');
var config = require('../../config/environment');
var testUtils = require('../testUtils.spec');

describe('GET /api/events', function() {
  var userid;
  var boat1, boat2, publicBoat;
  var publicEvent;
  var testUser2;

  before(function(done) {
    testUtils.addTestUser('test2')
    .then(function(user2) { testUser2 = user2; })

    .then(function() { return testUtils.addTestUser('test'); })
    .then(function(user) {
        userid = user.id;
        return Q.all([
              testUtils.addTestBoat({name: "test boat 1", admins: [ user.id ]}),
              testUtils.addTestBoat({name: "test boat 2", readers: [ user.id ]}),
              testUtils.addTestBoat({name: "test boat public", publicAccess: true})
        ])
      })
    .spread(function (boat1_, boat2_, publicBoat_) {
         boat1 = boat1_;
         boat2 = boat2_;
         publicBoat = publicBoat_;
         
         (new Event({
           author: testUser2.id,
           boat: publicBoat,
           comment: 'Public comment',
           when: new Date()
         })).save(function(err, e) {
           if (err) { throw(new Error(err)); }
           publicEvent = e;
           done();
         });
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

  var noteid;

  it('should add a simple note', function(done) {
     server
      .post('/api/events')
      .set('Authorization', 'Bearer ' + token)
      .send({
        author: userid,
        boat: boat1.id,
        comment: 'test note',
        when: new Date("2015-01-01")})
      .expect(201)
      .end(function(err, res) {
        if (err) return done(err);
        res.body.should.have.property('_id');
        noteid = res.body._id;
        done();
      });
  });

  it('should retrieve a note by id', function(done) {
     server
      .get('/api/events/' + noteid)
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
        if (err) return done(err);
        res.body.should.have.property('comment');
        res.body.comment.should.equal('test note');
        done();
      });
  });

  it('should retrieve a note from a public boat without auth', function(done) {
     server
      .get('/api/events/' + publicEvent._id)
      .expect(200)
      .end(function(err, res) {
        if (err) return done(err);
        res.body.should.have.property('comment');
        res.body.comment.should.equal('Public comment');
        done();
      });
  });

  it('should list the test note and the public note', function(done) {
     server
      .get('/api/events')
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
        if (err) {
           return done(err);
        }
        res.body.should.be.instanceof(Array);
        res.body.should.have.length(2);
        res.body[0].should.have.property('comment');
        res.body[1].should.have.property('comment');
        res.body[0]._id.should.not.equal(res.body[1]._id);
        done();
      });
  });

  it('should not list the old test note', function(done) {
     server
      .get('/api/events?A=2015-02-01')
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
        if (err) {
           return done(err);
        }
        res.body.should.be.instanceof(Array);
        res.body.should.have.length(1);
        res.body[0].should.have.property('comment');
        res.body[0].comment.should.equal('Public comment');
        done();
      });
  });

  it('should refuse to add a note on an invalid boat', function(done) {
     server
      .post('/api/events')
      .set('Authorization', 'Bearer ' + token)
      .send({author: userid, boat: 'invalid', comment: 'test note', when: new Date()})
      .expect(400)
      .end(function(err, res) {
        if (err) return done(err);
        done();
      });
  });

  it('should refuse to add a note on an observed boat', function(done) {
     server
      .post('/api/events')
      .set('Authorization', 'Bearer ' + token)
      .send({author: userid, boat: boat2.id, comment: 'test note', when: new Date()})
      .expect(403)
      .end(function(err, res) {
        if (err) return done(err);
        done();
      });
  });

  var photoName = '39AA3032-0C2B-4EFC-9DDD-60ED13B07B98.jpg';
  var photoPath = __dirname + '/' + photoName;
  it('should accept a photo upload', function(done) {
    server
      .post('/api/events/photo/' + boat1.id)
      .set('Authorization', 'Bearer ' + token)
      .attach(photoName, photoPath)
      .send()
      .expect(201)
      .end(done);
  });

  it('should serve a photo', function(done) {
    server
      .get('/api/events/photo/' + boat1.id + '/' + photoName)
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(done);
  });

  it('should refuse to serve a photo without auth', function(done) {
    server
      .get('/api/events/photo/' + boat1.id + '/' + photoName)
      .expect(401)
      .end(done);
  });

  it('should refuse a photo upload', function(done) {
    server
      .post('/api/events/photo/' + boat2.id)
      .set('Authorization', 'Bearer ' + token)
      .attach(photoName, photoPath)
      .send()
      .expect(403)
      .end(done);
  });


  after(function(done) {
    testUtils.cleanup();
    Event.remove({author: userid}, done);

    // cleanup the uploaded photo.
    rmdir(config.uploadDir + '/photos/' + boat1.id, function() { });
  });
});
