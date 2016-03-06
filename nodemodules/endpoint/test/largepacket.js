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


function sendLargePacket(src, dst, label, buf, settings, cb) {
  var splitCount = largepacket.splitBuffer(buf, settings.chunkSizeBytes).length;
  var expectedCount = (splitCount <= 1? 1 : 1 + splitCount);
  endpoint.tryMakeAndResetEndpoint(
    '/tmp/' + src + '.db', src, function(err, ep) {
      assert(!err);      
      assert(ep);
      ep.getTotalPacketCount(function(err, n) {
        assert(!err);
        assert(n == 0);
        largepacket.sendPacket(dst, ep, label, buf, settings, function(err) {
          assert(!err);
          ep.getTotalPacketCount(function(err, n) {
            assert(n == expectedCount);

            // At this stage, the large packet has been posted
            // at the sending endpoint

            cb(err, ep);
          });
        });
      });
    });
}

function testSendPacketWithData(src, dst, buf, done) {
    var label = 23;
    var settings = {chunkSizeBytes: 8};
    var partsDir = '/tmp/packetparts/';

    var result = Q.defer(); // Will be resolved to the final large packet
    result.promise.then(function(packet) {
      assert(packet.dst == dst);
      assert(packet.label == label);
      assert(packet.data.equals(buf));
      assert(packet.src == src);
      fs.readFile(Path.join(
        partsDir, 
        src + '_' + packet.seqNumber, 'first.dat'), function(err) {
          // We expect there to be an error reading a non-existing file:
          assert(err); 
          done();
      });
    }).done();

    sendLargePacket(src, dst, label, buf, settings, function(err, sender) {
      assert(!err);
      assert(sender);

      endpoint.tryMakeAndResetEndpoint(
        '/tmp/largepacketreceiver', 
        dst, function(err, receiver) {


          // Packet handlers for large packets are added the same
          // way as for small packets
          receiver.addPacketHandler(function(endpoint, packet) {
            if (packet.label == label) {
              result.resolve(packet);
            }
          });

          // We have to remember to add this handler if the receiver should
          // be able to receive large packets. partsDir is the location where
          // the parts of packets are temporarily stored while it is being received.
          receiver.addPacketHandler(largepacket.makeLargePacketHandler(partsDir));

          sync2.synchronize(sender, receiver, function(err) {
            result.reject('Failed to synchronize');
          });
        })

    });
}


describe('largepacket', function() {
  it(
    'Should split a large packet into smaller ones, and encode/decode the small packets',
    function() {
      var label = 23;
      var data = makeLargePacket(60);
      var buf = msgpack.encode(data);
      var settings = {chunkSizeBytes: 8};
      var splitted = largepacket.splitBuffer(buf, settings.chunkSizeBytes);
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
    testSendPacketWithData('largesend', 'katt', buf, done);
  });

  it('Should send a small packet', function(done) {
    var buf = new Buffer(1);
    buf[0] = 119;
    testSendPacketWithData('smallsend', 'katt2', buf, done);
  });
});
