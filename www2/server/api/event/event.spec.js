'use strict';

var should = require('should');
var app = require('../../app');
var request = require('supertest');
var User = require('../user/user.model');
var Boat = require('../boat/boat.model');
var Event = require('./event.model');
var Q = require('q');

function addTestBoat(boat) {
  return Q.Promise(function(resolve, reject) {
    var testBoat = new Boat(boat);
    testBoat.save(function(err, boat) {
      if (err) {
        reject(err);
      } else {
        resolve(boat);
      }
    });
  });
}

describe('GET /api/events', function() {
  var userid;
  var boat1, boat2;
  before(function(done) {
    var testUser = new User({
      "provider" : "local",
      "name" : "test",
      "email" : "test@anemomind.com",
      "hashedPassword" : "bj0zHvlC/YIzEFOU7nKwr+OHEzSzfdFA9PMmsPGnWITGHp1zlL+29oa049o6FvuR2ofd8wOx2nBc5e2n2FIIsg==",
      "salt" : "bGwuseqg/L/do6vLH2sPVA==",
      "role" : "user"
    });
    testUser.save(function(err, user) {
      userid = user.id;
      Q.all([
            addTestBoat({name: "test boat 1", admins: [ user.id ]}),
            addTestBoat({name: "test boat 2", readers: [ user.id ]})
      ]).spread(function (boat1_, boat2_) {
         boat1 = boat1_;
         boat2 = boat2_;
         done();
      });
    });
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

  var noteid;

  it('should add a simple note', function(done) {
     server
      .post('/api/events')
      .set('Authorization', 'Bearer ' + token)
      .send({author: userid, boat: boat1.id, comment: 'test note', when: new Date()})
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

  it('should list the test note', function(done) {
     server
      .get('/api/events')
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
        if (err) {
           return done(err);
        }
        res.body.should.be.instanceof(Array);
        res.body.should.have.length(1);
        res.body[0].should.have.property('comment');
        res.body[0].comment.should.equal('test note');
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
  after(function(done) {
    Boat.remove({name: "test boat"}).exec();
    User.remove({email: "test@anemomind.com"}).exec();
    Event.remove({author: userid}, done);
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
  after(function(done) {
    Boat.remove({name: boat1.name}).exec();
    Boat.remove({name: boat2.name}).exec();
    User.remove({email: "test@anemomind.com"}).exec();
    Event.remove({author: userid}, done);
  });
});
