var epstate = require('../epstate.js');
var ep = require('../endpoint.sqlite.js');
var assert = require('assert');

describe('epstate', function() {
  it('Should get the state of an empty endpoint', function(done) {
    ep.tryMakeAndResetEndpoint('/tmp/epstateA.db', 'A', function(err, ep) {
      epstate.getAllEndpointData(ep, function(err, data) {
        assert(data.packets.length == 0);
        assert(data.lowerBounds.length == 0);
        done();
      });
    });
  });

  it('Should show the state of an endpoint before and after an operation', 
     function(done) {
    ep.tryMakeAndResetEndpoint('/tmp/epstateA.db', 'A', function(err, ep) {
      epstate.getAllEndpointData(ep, function(err, data) {
        epstate.dispStateOp(ep, function(cb) {
          console.log('sendPacket');
          ep.sendPacket('B', 3, new Buffer(119), cb);
        }, done);
      });
    });
  });

  it('Should get the state of an endpoint with some data', function(done) {
    ep.tryMakeAndResetEndpoint('/tmp/epstateA.db', 'A', function(err, ep) {
      ep.sendPacket('B', 3, new Buffer(1), function(err) {
        if (err) {return done(err);}
        ep.sendPacket('B', 3, new Buffer(2), function(err) {
          if (err) {return done(err);}
          ep.sendPacket('A', 4, new Buffer(2), function(err) {
            if (err) {return done(err);}
            ep.updateLowerBound('C', 'D', "000001597313f7ff", function(err) {
              if (err) {return done(err);}
              epstate.getAllEndpointData(ep, function(err, data) {
                if (err) {return done(err);}
                var sum = epstate.summarizeEndpointData(data);
                console.log("sum: " + JSON.stringify(sum, null, 2));
                var packets = sum.packets;
                for (var i = 0; i < packets.length; i++) {
                  var p = packets[i];
                  assert(typeof p.minSeqNumber == "string");
                  assert(typeof p.maxSeqNumber == "string");
                }
                assert(sum.packets.length == 2);
                assert(sum.lowerBounds.length == 1);
                done();
              });
            });
          });
        });
      });
    });
  });
});
