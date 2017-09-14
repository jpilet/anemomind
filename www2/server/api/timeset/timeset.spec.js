'use strict'

var app = require('../../app');
var request = require('supertest');
var Timeset = require('./timeset.model.js');
var User = require('../user/user.model.js');
var Boat = require('../boat/boat.model.js');
var mongoose = require('mongoose');
var assert = require('assert');
var Schema = mongoose.Schema;
var utils = require('../testUtils.spec.js');
var assert = require('assert');

var d = new Date();

function sameDates(a, b) {
  var ad = new Date(a);
  var bd = new Date(b);
  if (ad.getTime() === bd.getTime()) {
    return true;
  } else {
    console.log("......... not same dates");
    console.log("ad");
    console.log(ad);
    console.log("bd");
    console.log(bd);
    return false;
  }
}

function prepareRecord(boat, cb) {
  Timeset.remove({}, function(err) {
    if (err) {
      cb(err);
    } else {
      var boatId = boat._id;
      var tsData = {
        boat: boatId,
        type: "delete",
        begin: d,
        end: d + 2000
      };
      var ts = new Timeset(tsData);
      ts.save(function(err, timesetId) {
        cb(err, timesetId, boatId);
      });
    }
  });
}


function prepareAll(cb) {
  User.remove({}, function(err) {
    if (err) {
      return cb(err);
    }
    utils.addTestUser("timesets").then(function(user) {
      utils.addTestBoat({
        name: "boat with timesets", 
        admins: [user._id]
      }).then(function(boat) {
        prepareRecord(boat, function(err, data) {
          cb(err, user, boat, data);
        });
      });
    });
  })
}

describe('////////////////// Timeset', function() {
  var server = request(app);
  var token;
  var user;
  var boat;
  var data;


  it('Acquire auth token for timesets and initialize everything', function(done) {
    prepareAll(function(err, u, b, d) {
      user = u;
      data = d;
      boat = b;
      if (err) {
        return done(err);
      } else if (!user) {
        return done(new Error("Failed to obtain user"));
      } else if (!data) {
        return done(new Error("Failed to create data"));
      } else if (!boat) {
        return done(new Error("Failed to obtain boat"));
      } else {
        server
          .post('/auth/local')
          .send({ email: 'timesets@test.anemomind.com', password: 'anemoTest' })
          .expect(200)
          .expect('Content-Type', /json/)
          .end(function (err, res) {
            token = res.body.token;
            return done(err);
          });
      }
    });
  });

  it('GET /api/timeset', function(done) {
    server
      .get('/api/timeset/' + boat._id)
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
        console.log("Got this: %j", res.body);
        var data = res.body;
        assert(data.length == 1);
        var x = data[0];
        assert(sameDates(x.begin, d));
        assert(sameDates(x.end, d + 2000));
        assert(x.type == "delete");
        done(err);
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
  

  after(function(done) {
    utils.cleanup();
    done();
  });

});

