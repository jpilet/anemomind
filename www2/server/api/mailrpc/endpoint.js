var mb = require('endpoint/endpoint.sqlite.js');
var packetCallbacks = require('./packet-callbacks.js');
var common = require('endpoint/common.js');
var naming = require('endpoint/naming.js');
var mkdirp = require('mkdirp');
var config = require('../../config/environment');
var path = require('path');
var schema = require('endpoint/endpoint-schema.js');
var boat = require('../boat/boat.controller.js');
var Boat = require('../boat/boat.model.js');
var boatAccess = require('../boat/access.js');

function openEndpoint(endpointName, cb) {
  if (!common.isValidEndpointName(endpointName)) {
    cb(new Error('Invalid endpoint name: ' + endpointName));
  } else {
    mkdirp(config.endpointDir, 0755, function(err) {
      if (err) {
        cb(err);
      } else {
        var filename = path.join(
          config.endpointDir,
          naming.makeDBFilename(endpointName));
        mb.tryMakeEndpoint(
          filename, endpointName,
          function(err, endpoint) {
	          if (err) {
	            cb(err);
	          } else {
              //schema.makeVerbose(endpoint);
	            packetCallbacks.add(endpoint);
	            cb(err, endpoint);
	          }
          });
      }
    });
  }
}

var testing = (process.env.NODE_ENV == 'development');


// Check if a user is authorized to access a endpoint.
// If not, produce an error object.
function acquireEndpointAccess(user, endpointName, cb) {
  var errorObject = {statusCode: 403,
		     message: "Unauthorized access to " + endpointName
                        + " by user: " + user.name + "(" + user.id + ")" };
  if (testing && endpointName == 'b') { // Used by iPhone unit tests.
    cb();
  } else {
    var parsed = naming.parseEndpointName(endpointName);
    if (parsed == null) {
      cb(errorObject);
    } else if (parsed.prefix == "boat") {
      Boat.findById(parsed.id, function (err, boat) {
        if (err) {
	  // Don't the error details to intruders. Should we log it?
	  cb(errorObject);
        } else {
	  if (boat) {
	    // I guess it makes sense to require write access.
	    if (boatAccess.userCanWrite(user, boat)) {
	      cb(); // OK, move on
	    } else {
	      cb(errorObject);
	    }
	  } else { // No such boat.
	    cb(errorObject);
	  }
        }
      });
    } else {
      cb(errorObject);
    }
  }
}

// cbOperation is a callback that receives as arguments
//   * the endpoint
//   * a callback to call once we are done working on the endpoint
// done is a callback that is called once the operation was done.
function withEndpoint(endpointName, req, cbOperation, done) {
  var user = req.user;
  acquireEndpointAccess(user, endpointName, function(err) {
    if (err) {
      done(err);
    } else {
      openEndpoint(endpointName, function(err, ep) {
        if (err) {
          done(err);
        } else {
          cbOperation(ep, function(err, result) {
            ep.close(function(err2) {
              done(err || err2, result);
            });
          });
        }
      });
    }
  });
}


exports.withEndpoint = withEndpoint;
