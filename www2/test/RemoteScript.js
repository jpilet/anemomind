var testPath = '/tmp/mailboxes/boat123456789012345678901234.mailsqlite.db';
var assert = require('assert');
var naming = require('endpoint/naming.js');
var BoxExec = require('../server/api/boxexec/boxexec.model.js');
var makeScriptResponseHandler = require('../server/api/boxexec/response-handler.js');
var common = require('../utilities/common.js');
var path = require('path');
var script = require('endpoint/script.js');
var mb = require('endpoint/endpoint.sqlite.js');
var sync = require('endpoint/sync2.js');
var mkdirp = require('mkdirp');

common.init();

var withTestBoat = require('./testboat.js');

function withConnectionAndTestBoat(cbOperation, cb) {
  withTestBoat(cbOperation, cb);
}

//function withConnectionAndBoat(cbOperation, cbDone)

function makeAndResetEndpoint(filename, endpointName, cb) {
  dir = path.parse(filename).dir;
  mkdirp(dir, 0755, function(err) {
    if (err) {
      cb(err);
    } else {
      mb.tryMakeEndpoint(filename, endpointName, function(err, endpoint) {
        if (err) {
          cb(err);
        } else {
          endpoint.reset(function(err2) {
            if (err2) {
              cb(err2);
            } else {
              cb(null, endpoint);
            }
          });
        }
      });
    }
  });
}

describe('RemoteScript', function() {
  it('Should parse the filename of endpoint', function() {
    var parsed = common.extractBoatIdFromFilename(testPath);
    assert(parsed == '123456789012345678901234');
  });

  // Just so that we can perform test cases.
  it('Should insert and remove a test boat. Check that there is an id', function(done) {
    withConnectionAndTestBoat(function(id, cb) {
      assert(id);
      assert(id.constructor.name == 'ObjectID');
      cb();
    }, done);
  });
  
  it('Should extract a box id', function(done) {
    withConnectionAndTestBoat(function(id, cb) {
      assert(id);
      common.getBoxIdFromBoatId(id, function(err, boxId) {
        assert(!err);
        assert.equal(boxId, 'abc119');
        cb();
      });
    }, done);
  });

  it('Should send a script for execution', function(done) {
    var scriptData = 'cd /tmp\npwd';
    withConnectionAndTestBoat(function(id, doneAll) {
      var boatId = id;
      var boatEndpointName = naming.makeEndpointNameFromBoatId(id);
      var filename = common.makeBoatDBFilename(boatId);

      
      // To be set later in the code.
      var performSync = null;
      var boxEndpointName = naming.makeEndpointNameFromBoxId('abc119');
      
      // Make a endpoint for the anemobox
      makeAndResetEndpoint(
        path.join('/tmp/', naming.makeDBFilename(boxEndpointName)),
        boxEndpointName, function(err, boxEndpoint) {
          assert(!err);
          assert(boxEndpoint);

          // Make a endpoint for the boat
          makeAndResetEndpoint(filename, boatEndpointName,
                              function(err, boatEndpoint) {
            assert(!err);

            // Called when the response of executing the script is coming back.
            boatEndpoint.addPacketHandler(makeScriptResponseHandler(
              function(err, response) {
                assert(!err);
                assert(response);
                assert(response._id);
                assert.equal(response.stdout, '/tmp\n');
                assert.equal(response.boxId, 'abc119');
                assert.equal(response.boatId, boatId);
                assert.equal(response.err, null);
                assert.equal(response.stderr, null);
                assert.equal(response.type, 'sh');
                assert.equal(response.script, scriptData);
                doneAll();
              }));

            // Send the script
            common.sendScriptToBox(boatId, 'sh', scriptData, function(err, data) {
              assert(!err);
              
              performSync = function() {
                sync.synchronize(boatEndpoint, boxEndpoint, function(err) {
                  assert(!err);
                });
              };
              
              boxEndpoint.addPacketHandler(script.makeScriptRequestHandler(performSync));

              // Run the first sync. This will propagate the script to the box,
              // that will execute it.
              performSync();
            });
          });
        });
    }, function(err) {
      assert(!err);
      done();
    });
  });
});



