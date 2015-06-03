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
          var msg = script.unpackScriptResponse;
          assert(msg.reqCode == reqCode);
          assert(msg.stdout == '/tmp');
          done();
        };
        script.runRemoteScript(mb0, 'b', 'sh', 'pwd', function(err, rc) {
          console.log(err);
          assert(!err);
          reqCode = rc;
          performSync(function(err) {
            assert(!err);
            common.strongLog('INITIATE SYNC...');
          });
        });
      });
    });
  });
});
