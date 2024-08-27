// A remote endpoint that we can play with, over HTTP.

import { ServerConnection } from './server-connection.js';
import { decode, encodeArgs, encodeGetArgs } from './json-coder.js';

// Make a method to put in the local endpoint object
// that will result in an HTTP request according
// to the schema.
function makeMethod(scon, endpointName, method) {
    return function() {
	var allArgs = Array.prototype.slice.call(arguments);
	var lastArgIndex = allArgs.length - 1;
	var args = allArgs.slice(0, lastArgIndex);
	var cb = allArgs[lastArgIndex];
	var responseHandler = function(err, data) {
	    cb(decode(method.output[0], err),
	       decode(method.output[1], data));
	};
      try {
	if (method.httpMethod == 'post') {
	    scon.makePostRequest(
		endpointName,
		method,
		encodeArgs(method.input, args),
		responseHandler
	    );
	} else {
	    if (args.length != method.input.length) {
                throw new Error("length mismatch");
            }
	    scon.makeGetRequest(
		endpointName,
		method,
		encodeGetArgs(method.input, args),
		responseHandler
	    );
	}
      } catch (e) {
        console.log('CAUGHT EXCEPTION WHEN SENDING REQUEST in httputils.js');
        console.log('INPUT SPEC');
        console.log(method.input);
        cb(e);
      }
    }
}

function Endpoint(schema, serverConnection, endpointName) {
  // TODO: remove this line once we have migrated.
  this.endpointName = endpointName;
  
  this.name = endpointName;
  for (let methodName in schema.methods) {
    this[methodName] = makeMethod(
      serverConnection,
      endpointName,
      schema.methods[methodName]
    );
  }
}

// Call this function when you need a new endpoint.
function tryMakeEndpoint(schema, serverAddress, userdata, endpointName, cb) {
    var s = new ServerConnection(serverAddress);
    s.login(userdata, function(err) {
	if (err) {
	    cb(err);
	} else {
	    // Register these as rpc calls.
	    cb(undefined, new Endpoint(schema, s, endpointName));
	}
    });
}

export function makeEndpointConstructor(schema) {
  return function(serverAddress, userData, endpointName, cb) {
    tryMakeEndpoint(schema, serverAddress, userData, endpointName, cb);
  };
}
