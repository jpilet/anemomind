var testserver = require('./testserver.js');
var assert = require('assert');
var chai = require('chai');
var chaihttp = require('chai-http');
var httpapi = require('../httpapi.js');
var endpoint = require('../endpoint.sqlite.js');
var binaryParser = require('superagent-binary-parser');
var express = require('express');

chai.use(chaihttp);

var app = testserver.app;

function encodeAndDecode(packet) {
  return httpapi.decodePacket(httpapi.encodePacket(packet).success).success;
}

function doEncodeAndDecodeWork(packet) {
  var packet2 = encodeAndDecode(packet);
  return (packet2.label == packet.label) && (packet2.data.equals(packet.data));
}

describe('httpapi', function() {
  it('should export an app', function() {
    assert(app);
  });

  it('test encode and decode', function() {
    assert(doEncodeAndDecodeWork({label: 3, data: new Buffer([0, 4, 7])}));
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

  it('bad-request-to-get-range-sizes', function(done) {
    endpoint.tryMakeAndResetEndpoint('/tmp/httpendpoint.db', 'a', function(err, ep) {
      if (err) {
        done(err);
      } else {

        var cleanup = function(err) {/*nothing to cleanup*/};
        app.use('/sqlite', httpapi.make(express.Router(), function(name, f) {
          if (name == ep.name) {
            f(null, ep, cleanup);
          } else {
            f(new Error('No such endpoint: ' + name), null, cleanup);
          }
        }));

        chai.request(app)
          .post('/sqlite/getRangeSizes/a')
          .send({'queries': [{src: 9, dst: 10, lower: 'asdfasf', upper: {a: 44}}]})
          .end(function(err, res) {
            assert(err);
            assert(res.status == 400);
            done();
          });
      }
    });
  });

  it('real-endpoint', function(done) {
    endpoint.tryMakeAndResetEndpoint('/tmp/httpendpoint.db', 'a', function(err, ep) {
      if (err) {
        done(err);
      } else {
        var cleanup = function(err) {/*nothing to cleanup*/};
        app.use('/sqlite', httpapi.make(express.Router(), function(name, f) {
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
                assert(res.status == 200);
                assert(res.header["content-length"] == 5);


                chai.request(app)
                  .get('/sqlite/getSummary/a')
                  .end(function(err, res) {
                    assert(res.status == 200);
                    assert(res.body);
                    assert(res.body.lowerBounds);
                    assert(res.body.packets);
                    assert(res.body.packets.length == 1);
                    var f = res.body.packets[0];
                    assert(f.src == 'a');
                    assert(f.dst == 'b');

                    var queries = res.body.packets;
                    chai.request(app)
                      .post('/sqlite/getRangeSizes/a')
                      .send({'queries': queries})
                      .end(function(err, res) {
                        assert(!err);
                        assert(res);
                        assert(res.status == 200);
                        assert(res.body.length == 1);
                        var x = res.body[0];
                        var q = queries[0];
                        assert(x.src == q.src);
                        assert(x.dst == q.dst);
                        assert(x.size == 4);

                        queries[0].src = q.dst;

                        chai.request(app)
                          .post('/sqlite/getRangeSizes/a')
                          .send({'queries': queries})
                          .end(function(err, res) {
                            assert(!err);
                            var x = res.body[0];
                            assert(x);
                            assert(x.size == 0);
                            done();
                          });                        
                      });
                  });
              });
          }
        });
      }
    });
  });
});
