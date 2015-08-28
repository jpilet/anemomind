process.env.ANEMOBOX_CONFIG_PATH = '/tmp/anemoboxcfg';

var app = require('../components/http');
var lep = require('../components/LocalEndpoint.js');
var request = require('supertest');
var server = request(app);
var should = require('should');
var assert = require('assert');

describe('httpsync', function() {
  it('Get the packet count', function(done) {
    lep.getName(function(lname) {
      server
        .post('/api/rpc/ep_getTotalPacketCount')
        .send({name: lname})
        .expect(200)
        .end(function(err, res) {
          console.log(err);
          assert(!err);
          assert(res);
          assert(res.body);
          var data = res.body;
          assert(data.result == 0);
          done();
        });
    });
  });

  it('Get the packet count for an endpoint that doesnt exist', function(done) {
    server
      .post('/api/rpc/ep_getTotalPacketCount')
      .send({name: 'badname'})
    // We expect 200 because the function was successfully called.
    // But its return value encodes an error.
      .expect(200) 
      .end(function(err, res) {
        console.log(err);
        assert(!err);
        assert(res);
        assert(res.body);
        var data = res.body;
        assert(!data.result);
        assert(data.error);
        done();
      });
  });

  it('Call a function that has not been registered', function(done) {
    server
      .post('/api/rpc/ep_functionThatHasNotBeenRegistered')
      .send({name: 'name', arg: 334})
      .expect(404)
      .end(function(err, res) {
        done();
      });
  });
});
