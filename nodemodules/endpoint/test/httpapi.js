var testserver = require('./testserver.js');
var assert = require('assert');
var chai = require('chai');
var chaihttp = require('chai-http');
var httpapi = require('../httpapi.js');
var endpoint = require('../endpoint.sqlite.js');

chai.use(chaihttp);

var app = testserver.app;

describe('httpapi', function() {
  it('should export an app', function() {
    assert(app);
  });

  it('should have some pine needle tea', function(done) {
    chai.request(app)
      .get('/')
      .end(function(err, res) {
        assert(res.status == 200);
        assert(res.text == "Pine needle tea");
        done();
      });
  });

  it('should access the endpoint', function(done) {
    chai.request(app)
      .get('/mockendpoint/info')
      .end(function(err, res) {
        assert(res.status == 200);
        assert(res.text == 'endpoint');
        done();
      });
  });

  it('encode packet', function() {
    var packetData = new Buffer([9, 0, 4]);
    var packet = {
      label: 119,
      data: packetData
    };

    var encoded = httpapi.encodePacket(packet);
    assert(encoded.success);
    assert(encoded.success.length == 4);
    var expected = [119, 9, 0, 4];
    for (var i = 0; i < 4; i++) {
      assert(expected[i] == encoded.success.readUInt8(i));
    }

    assert(httpapi.encodePacket(9).failure);
    assert(httpapi.encodePacket({label: 4}).failure);
    assert(httpapi.encodePacket({data: new Buffer(4)}).failure);
    assert(httpapi.encodePacket({label: 4, data: 'asdfa'}).failure);
  });

  it('real-endpoint', function(done) {
    endpoint.tryMakeEndpoint('/tmp/httpendpoint.db', 'pine', function(err, ep) {
      if (err) {
        done(err);
      } else {

        app.use('/sqlite', httpapi.make(function(f) {
          f(ep, function(err) {});
        }));

        done();

      }
    });
  });
});
