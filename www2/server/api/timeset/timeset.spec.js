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

var c = new Date();
var d = new Date(c.getTime() + 4000);

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
        lower: d,
        upper: d + 2000,
        data: "Kattskit",
        creationTime: c
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
  var idToRemove;


  it('Acquire auth token for timesets and initialize everything', function(done) {
    prepareAll(function(err, u0, b0, d0) {
      user = u0;
      data = d0;
      boat = b0;
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
        var data = res.body;
        assert(data.length == 1);
        var x = data[0];
        assert(sameDates(x.lower, d));
        assert(sameDates(x.creationTime, c));
        assert(x.data == "Kattskit");
        assert(sameDates(x.upper, d + 2000));
        assert(x.type == "delete");
        done(err);
      });
  });

  it('POST /api/timeset', function(done) {
    server
      .post('/api/timeset/' + boat._id)
      .set('Authorization', 'Bearer ' + token)
      .send({
        boat: boat._id,
        type: "ignore",
        lower: d + 1000,
        upper: d + 3000
      })
      .expect(200)
      .expect('Content-Type', /json/)
      .end(function(err, res) {
        idToRemove = res.body._id;
        assert(idToRemove);
        assert(res.body.boat == boat._id);
        done(err);
      });
  });

  it('GET /api/timeset a second time', function(done) {
    server
      .get('/api/timeset/' + boat._id)
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
        var data = res.body;

        assert(data.length == 2);

        for (var key in data) {
          var x = data[key];
          if (sameDates(x.lower, d + 1000)) {
            assert(sameDates(x.upper, d + 3000));
            assert(x.type == "ignore");
          }
        }
        done(err);

      });
  });

  it('DELETE /api/timeset', function(done) {
    server.delete('/api/timeset/' + boat._id + '/' + idToRemove)
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(done);
  });
  
  it('DELETE /api/timeset nonexistant', function(done) {
    server.delete('/api/timeset/' + boat._id + '/deadbeef')
      .set('Authorization', 'Bearer ' + token)
    // "deadbeef" cannot be a valid mongoose.Types.ObjectId, 
    // so an exception will be thrown which results in this 
    // code later down the chain...
      .expect(500) 
      .end(done);
  });

  it('GET /api/timeset', function(done) {
    server
      .get('/api/timeset/' + boat._id)
      .set('Authorization', 'Bearer ' + token)
      .expect(200)
      .end(function(err, res) {
        var data = res.body;
        assert(data.length == 1);
        var x = data[0];
        assert(sameDates(x.lower, d));
        assert(sameDates(x.upper, d + 2000));
        assert(x.type == "delete");
        done(err);
      });
  });

  after(function(done) {
    utils.cleanup();
    done();
  });

});

