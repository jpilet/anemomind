// This file exports a single function, register, that can be used to fille the rpcFuncTable of
// rpcble.js. It is used by RpcEndpoint.js. The reason for putting this code in its own file
// is to facilitate unit testing.
var schema = require('endpoint/endpoint-schema.js');
var coder = require("endpoint/json-coder.js");
var mb = require("./LocalEndpoint.js");

// Conveniency function for
// error handling.
//
// If p evaluates to true, this function returns true without any side effects.
// Otherwise, it returns false and calls cb with the error message wrapped inside
// a JSON object.
function ensureCB(p, errorMessage, cb) {
  if (p) {
    return true;
  } else {
    cb({error: errorMessage});
    return false;
  }
}

/*
  TODO: Every time we access the endpoint, we open it, call the method and close it.
  If we want, we might want to leave the last opened endpoint opened until we call a
  method for a different endpoint.
*/
function callEndpointMethod(endpointName, methodName, args, cbFinal) {
  // TODO: Since there is only one endpoint endpoint on the
  // anemobox, maybe we could simply remove 'name'
  // from the RPC protocol? In that case, we would simply
  // use mb.open(...) below.
  mb.withNamedLocalEndpoint(endpointName, function(endpoint, cb) {
    var method = endpoint[methodName];
    if (!method || typeof(method) != 'function') {
      cb('unknown endpoint method: ' + endpoint.methodName);
    } else {
      try {
	      method.apply(endpoint, args.concat([
	        function(err, result) {
	          if (err) {
	            cb(err);
	          } else {
              cb(null, result);
	          }
	        }
	      ]));
      } catch (e) {
	      console.log(
	        "Please don't throw exceptions, passing errors to the callback is better.");
	      console.log("This exception was caught: %j", e);
	      cb(e);
      }
    }
  }, cbFinal);
}

function encodeResult(argSpecs, result) {
  var len = argSpecs.length;
  if (len == 1 || len == 2) {
    if (len == 1) {
      return {};
    } else {
      return {result: coder.encode(argSpecs[1], result)};
    }
  } else {
    return {error: "Bad arg specs"};
  }
}

// Here we make a function that takes an incoming
// JSON object, decodes it, call a method on a local
// endpoint and return the result.
function makeRpcFunction(methodName, method) {
  return function(data, cb) {
    mb.getName(function(localName) {
      var endpointName = data.name;
      if (typeof endpointName != 'string') {
        cb({error: 'The endpoint name must be a string'});
      } else if (localName.trim() != endpointName.trim()) {
        cb({error: 'The local endpoint is named "' + localName + '" but you are attempting to access "' + endpointName + '"'});
      } else {
        try {
          if (ensureCB(endpointName != undefined,
		       "You must pass a endpoint name", cb)) {
	    var args = coder.decodeArgs(method.input, data);
	    callEndpointMethod(
	      endpointName, methodName, args,
	      function(err, result) {
	        if (err) {
	          var message = "Error accessing endpoint with name " +
		    endpointName + " and method " + methodName;
	          console.log(message);
	          console.log("The error is %j", err);
	          cb({error: message +  + ". See the server log for details."});
	        } else {
	          cb(encodeResult(method.output, result));
	        }
	      }
	    );
          }
        } catch (e) {
          console.log("Caught this exception: " + e);
          cb({error: "Caught an exception on the server. See the server log for details."});
        }
      }
    });
  }
}

// Prefix all endpoint-related calls with mb
// to avoid naming collisions for common names (such as "reset")
function makeRpcFuncName(methodName) {
  return "ep_" + methodName;
}

// Use this function to register all the available endpoint calls
// that we serve
function register(rpcFuncTable) {
  for (var methodName in schema.methods) {
    var rpcFuncName = makeRpcFuncName(methodName);
    rpcFuncTable[rpcFuncName] = makeRpcFunction(
      methodName,
      schema.methods[methodName]
    );
  }
}

module.exports.register = register;
