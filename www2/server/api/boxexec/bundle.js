var Q = require('q');
var Path = require('path');
var script = require('endpoint/script.js');
var fs = require('fs');

function compileBundleScript(remoteBundleFilename, remoteDstPath) {
  return Q.nfcall(fs.readFile, Path.join(__dirname, 'deploybundle.js'), 'utf8')
    .then(function(s) {
      return s + '\nmodule.exports.main = makeMainFunction("' 
        + remoteBundleFilename + '", "' + remoteDstPath + '");';
    });
}

// Posts a script request on 'endpoint' to be sent
// to an endpoint named 'dstEndpointName'. That script
// will unpack the bundle assumed to be located at
// 'remoteBundleFilename' to the path 'remoteDstPath'.
// 'reqCode' is used to identify the request.
function deployRemoteBundle(
  endpoint, dstEndpointName, 
  reqCode,
  remoteBundleFilename, remoteDstPath, cb) {
  return compileBundleScript(remoteBundleFilename, remoteDstPath)
    .then(function(codeToExecute) {
      script.runRemoteScript(endpoint, dstEndpointName, 'js',
                             codeToExecute, reqCode, cb);

    }).catch(function(err) {
      cb(err);
    }).done();
}


module.exports.compileBundleScript = compileBundleScript;
module.exports.deployRemoteBundle = deployRemoteBundle;
