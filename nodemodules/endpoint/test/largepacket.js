var largepacket = require('../largepacket.js');
var assert = require('assert');
var msgpack = require('msgpack-js');
var endpoint = require('../endpoint.sqlite.js');
var sync2 = require('../sync2.js');
var fs = require('fs');
var Q = require('q');
var Path = require('path');

function makeLargePacket(n) {
  var dst = new Array(n);
  for (var i = 0; i < n; i++) {
    dst[i] = i*i + 0.3;
  }
  return dst;
}


function sendLargePacket(hide, src, dst, label, buf, mtu, cb) {
  assert(typeof mtu == 'number');
  var splitCount = largepacket.splitBuffer(buf, mtu).length;
  var expectedCount0 = 1 + splitCount;
  var expectedCount = (hide && splitCount == 1? 1 : expectedCount0);
  endpoint.tryMakeAndResetEndpoint(
    '/tmp/' + src + '.db', src, function(err, ep) {
      ep.settings.mtu = mtu;
      assert(!err);      
      assert(ep);
      ep.getTotalPacketCount(function(err, n) {
        assert(!err);
        assert(n == 0);

        var sendPacketMethod = (hide? function(cb0) {
          assert(typeof cb0 == 'function');
          ep.sendPacket(dst, label, buf, cb0);
        } : function(cb0) {
          assert(typeof cb0 == 'function');
          largepacket.sendPacket(ep, dst, label, buf, ep.settings, cb0);
        });

        console.log('METHOD: ' + (hide? "ep.sendPacket" : "largepacket.sendPacket"));

        sendPacketMethod(function(err) {
          assert(!err);
          ep.getTotalPacketCount(function(err, n) {

            console.log('n = ' + n);
            assert(n == expectedCount);

            // At this stage, the large packet has been posted
            // at the sending endpoint

            cb(err, ep);
          });
        });
      });
    });
}

function testSendPacketWithData(hide, src, dst, buf, done) {
  var label = 23;
  var result = Q.defer(); // Will be resolved to the final large packet

  var partsPath = '';

  sendLargePacket(hide, src, dst, label, buf, 8, function(err, sender) {
    assert(!err);
    assert(sender);

    endpoint.tryMakeAndResetEndpoint(
      '/tmp/largepacketreceiver', 
      dst, function(err, receiver) {

        partsPath = receiver.getPartsPath();

        // Packet handlers for large packets are added the same
        // way as for small packets
        receiver.addPacketHandler(function(endpoint, packet) {
          if (packet.label == label) {
            result.resolve(packet);
          }
        });

        sync2.synchronize(sender, receiver, function(err) {
          result.reject('Failed to synchronize');
        });
      })
  });

  result.promise.then(function(packet) {
    assert(packet.dst == dst);
    assert(packet.label == label);
    var data = packet.data;
    assert(data instanceof Buffer); // The filename of the saved data
    assert(packet.src == src);
    assert(data.equals(buf));
    done();
  }).done();
}


describe('largepacket', function() {
  it(
    'Should split a large packet into smaller ones, and encode/decode the small packets',
    function() {
      var label = 23;
      var data = makeLargePacket(60);
      var buf = msgpack.encode(data);
      var settings = {maxPacketSize: 8};
      var splitted = largepacket.splitBuffer(buf, settings.maxPacketSize);
      assert(splitted instanceof Array);
      var summedSizes = splitted
          .map(function(x) {return x.length;})
          .reduce(function(a, b) {return a + b;});
      assert(buf.length == summedSizes);
      assert(1 < splitted.length);
      
      var firstPacket = largepacket.encodeFirstPacket(label, splitted.length);
      assert(firstPacket.length == 8);

      var decodedFirst = largepacket.decodeFirstPacket(firstPacket);
      assert(decodedFirst);
      assert(decodedFirst.label == 23);
      assert(decodedFirst.count == splitted.length);

      var seqNumber = '2134324';

      var remainingPacket = largepacket.encodeRemainingPacket(seqNumber, splitted[1]);
      assert(remainingPacket.length == 4 + 7 + splitted[1].length);
      var decodedRemaining = largepacket.decodeRemainingPacket(remainingPacket);
      assert(decodedRemaining.seqNumber == seqNumber);
      assert(decodedRemaining.data instanceof Buffer);
      assert(splitted[1] instanceof Buffer);
      assert(decodedRemaining.data.equals(splitted[1]));
    });

  it('Should send a large packet', function(done) {
    var data = makeLargePacket(4);
    var buf = msgpack.encode(data);
    testSendPacketWithData(false, 'largesend', 'katt', buf, done);
  });

  it('Should send a small packet', function(done) {
    var buf = new Buffer(1);
    buf[0] = 119;
    testSendPacketWithData(false, 'smallsend', 'katt2', buf, done);
  });

  it('Should send a large packet', function(done) {
    var data = makeLargePacket(4);
    var buf = msgpack.encode(data);
    testSendPacketWithData(true, 'largesend', 'katt', buf, done);
  });

  it('Should send a small packet', function(done) {
    var buf = new Buffer(1);
    buf[0] = 119;
    testSendPacketWithData(true, 'smallsend', 'katt2', buf, done);
  });
});
