var script = require('../script.js');
var mb = require('../mail.sqlite.js');
var assert = require('assert');
var sync = require('../sync.js');
var common = require('../common.js');

describe('script', function() {
  it('Should execute a shell script', function(done) {
    mb.tryMakeMailbox('/tmp/mb0.db', 'a', function(err, mb0) {
      assert(!err);
      mb.tryMakeMailbox('/tmp/mb1.db', 'b', function(err, mb1) {
        assert(!err);

        var performSync = function(cb) {
          sync.synchronize(mb0, mb1, cb || function() {});
        }

        mb1.onPacketReceived = script.makeScriptRequestHandler(performSync);

        var reqCode = undefined;
        
        mb0.onPacketReceived = function(mailbox, packet, T, cb) {
          cb();
          assert(packet.label == common.scriptResponse);
          var msg = script.unpackScriptResponse(packet.data);
          assert.equal(msg.reqCode, reqCode);
          assert(msg.stdout == '/tmp\n');
          done();
        };
        script.runRemoteScript(mb0, 'b', 'sh', 'cd /tmp\npwd', function(err, rc) {
          assert(!err);
          reqCode = rc;
          performSync(function(err) {
            assert(!err);
          });
        });
      });
    });
  });
  
  it('Should execute a node script', function(done) {
    mb.tryMakeMailbox('/tmp/mb0.db', 'a', function(err, mb0) {
      assert(!err);
      mb.tryMakeMailbox('/tmp/mb1.db', 'b', function(err, mb1) {
        assert(!err);

        var performSync = function(cb) {
          sync.synchronize(mb0, mb1, cb || function() {});
        }

        mb1.onPacketReceived = script.makeScriptRequestHandler(performSync);

        var reqCode = undefined;

        var fib = function(x) {return (x < 2? x : fib(x-1) + fib(x-2));};
        mb0.onPacketReceived = function(mailbox, packet, T, cb) {
          cb();
          assert(packet.label == common.scriptResponse);
          var msg = script.unpackScriptResponse(packet.data);
          assert(msg.reqCode == reqCode);
          assert.equal(msg.result, fib(5));
          done();
        };
        script.runRemoteScript(
          mb0, 'b', 'js',
          'var fs = require("fs"); (function(cb) {var fib = function(x) {return (x < 2? x : fib(x-1) + fib(x-2));}; cb(null, fib(5));})',
          function(err, rc) {
            assert.equal(err, null);
            reqCode = rc;
            performSync(function(err) {
              assert(!err);
            });
          });
      });
    });
  });
});
