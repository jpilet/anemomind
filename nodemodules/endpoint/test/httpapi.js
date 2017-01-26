var testserver = require('./testserver.js');
var assert = require('assert');
var chai = require('chai');
var chaihttp = require('chai-http');
var httpapi = require('../httpapi.js');
var endpoint = require('../endpoint.sqlite.js');
var binaryParser = require('superagent-binary-parser');

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
  
  it('mock-endpoint', function(done) {
    chai.request(app)
      .get('/mockendpoint/getPacket/mock/a/b/deadbeef')
      .end(function(err, res) {
        assert(res.status == 200);
        chai.request(app)
          .get('/mockendpoint/getPacket/mock/c/d/deadbeef')
          .end(function(err, res) {
            assert(res.status == 500);
            done();
          });
      })
  });

  it('real-endpoint', function(done) {
    endpoint.tryMakeAndResetEndpoint('/tmp/httpendpoint.db', 'a', function(err, ep) {
      if (err) {
        done(err);
      } else {
        var cleanup = function(err) {/*nothing to cleanup*/};
        app.use('/sqlite', httpapi.make(function(name, f) {
          if (name == ep.name) {
            f(null, ep, cleanup);
          } else {
            f(new Error('No such endpoint: ' + name), null, cleanup);
          }
        }));

        var testData = new Buffer([3, 5, 8, 13]);
        
        ep.sendPacketAndReturn('b', 119, testData, function(err, packet) {
          if (err) {
            done(err);
          } else {
            chai.request(app)
              .get('/sqlite/getPacket/a/a/b/' + packet.seqNumber)

// I really don't know how to get the binary data of a response
// with Chai.
              .parse(binaryParser) 

              .end(function(err, res) {
                console.log('GOT: %j', res.data);
                assert(res.status == 200);
                assert(res.header["content-length"] == 5);
                done();
              });
          }
        });
      }
    });
  });
});
