var schemautils = require('./schemautils.js');
var schema = require('./endpoint-schema.js');
var coder = require('./json-coder.js');
var assert = require('assert');
var naming = require('./naming.js');

// This function is common, irrespective of whether it is a post or get request.
function callEndpointMethod(withEndpointAccess, endpointName, req, methodName, args, cb) {
  withEndpointAccess(endpointName, req, function(endpoint, done) {
    try {
      endpoint[methodName].apply(
	endpoint, args.concat([done])
      );
    } catch (e) { 
      console.log("Caught an exception when calling endpoint method: %j.", e);
      console.log("THIS SHOULD NOT HAPPEN and this is a bug. Please don't throw exceptions,");
      console.log("but pass the errors as the first argument to the callback instead.")
      done(e);
    }
  }, cb);
}

////////////////////////////////////////////////////////////////////////////////
/// HTTP interface
////////////////////////////////////////////////////////////////////////////////
// http://stackoverflow.com/questions/18391212/is-it-not-possible-to-stringify-an-error-using-json-stringify
// Code that lets us serialize Error objects
// to be passed back as JSON over HTTP and
// handled by the client.
Object.defineProperty(Error.prototype, 'toJSON', {
  value: function () {
    var alt = {};

    Object.getOwnPropertyNames(this).forEach(function (key) {
      alt[key] = this[key];
    }, this);

    return alt;
  },
  configurable: true
});

function getStatusCode(err) {
  if (err) {
    if (typeof err == 'object') {
      if (err.statusCode) {
	
	// In case of error, we can be specific about the status code.
	return err.statusCode;
      }
    }
    
    // Default code for general, unspecified error:
    return 500;
  } else {
    return 200; // OK
  }
}

// This will handle an HTTP request related to a specific method.
function handler(withEndpointAccess, method, req, res) {
  assert(method.httpMethod == 'post' || method.httpMethod == 'get');
  try {
    var resultCB = function(err, result) {
      var code = getStatusCode(err);

      // Do we need a try statement in this function?
      if (err) {
	console.log('WARNING: There was an error on the server: %j', err);
	console.log('THE CODE IS %j', code);
      }
      
      res.status(code).json(
	coder.encode(
	  method.output[err? 0 : 1], // How the return value should be coded.
	  (err? err : result) // What data to send.
	)
      );
    };
    var endpointName = req.params.endpointName;
    var args = null;

    if (method.httpMethod == 'post') {
      args = coder.decodeArgs(method.input, req.body);
    } else {
      args = coder.decodeGetArgs(method.input, req.params);
    }
    
    callEndpointMethod(
      withEndpointAccess,
      endpointName,
      req,
      method.name,
      args,
      resultCB
    );
  } catch (e) {
    resultCB(e);
  }
};

function makeHandler(withEndpointAccess, method) {
  return function(req, res) {
    handler(withEndpointAccess, method, req, res);
  };
}

// The basic path is just the method name together with the
// endpoint name.
function makeBasicSubpath(method) {
  return '/' + method.name + '/:endpointName';
}

// This is a general method for both POST and GET request.
// For GET requests, it adds a pattern for the arguments.
function makeSubpath(method) {
  return makeBasicSubpath(method) +
    (method.httpMethod == 'post' || method.httpMethod == 'put'? '' :

     // Also add a pattern for the arguments.
     coder.makeGetArgPattern(method.input));
}

// Adds a route to the router for a method.
function bindMethodHandler(withEndpointAccess, router, authenticator, method) {
  assert(schemautils.isValidHttpMethod(method.httpMethod));
  if (authenticator) {
    router[method.httpMethod](
      makeSubpath(method),
      authenticator,
      makeHandler(withEndpointAccess, method)
    );
  } else {
    router[method.httpMethod](
      makeSubpath(method),
      makeHandler(withEndpointAccess, method)
    );
  }
}

// Adds all routes to the router.
function bindMethodHandlers(withEndpointAccess, router, authenticator) {
  // Register a GET or POST handler
  // for every remote function that
  // we can call.
  for (var methodName in schema.methods) {
    bindMethodHandler(
      withEndpointAccess,
      router, authenticator, schema.methods[methodName]);
  }
}

// Prints a summary of the HTTP call to a remote function.
function makeMethodDesc(method) {
  console.log('Function name: %s', method.name);
  console.log('HTTP-call: %s %s\n', method.httpMethod.toUpperCase(), makeSubpath(method));
}

// To auto-generate a documentation.
function makeOverview() {
  for (var key in schema.methods) {
    makeMethodDesc(schema.methods[key]);
  }
}

module.exports.bindMethodHandlers = bindMethodHandlers;
module.exports.makeOverview = makeOverview;
