/*

= HOW TO USE =

== ON THE SENDER SIDE ==
bundle.sendBundle(dst, bundleSetup);

where bundleSetup is a map: {
 main: [path to file that should be run, relative to root. Can be .sh or .js]
 dst: [path where the bundle should be unpacked]
 src: [path to .bundle file]
}

== ON THE RECEIVER SIDE ==
The receiver needs to add a bundle handler among the packet handler
of the endpoint:

endpoint.addPacketHandler(bundle.makeBundleHandler());

*/

var mkdirp = require('mkdirp');
var Q = require('q');



function writeBundleToTempFile(data) {
  
}

function makeBundleHandler(reposPath) {
  return function(endpoint, packet) {
    if (packet.label == common.bundle) {
      var data = packet.data;
      if (data instanceof Buffer) {
        writeBundleToTempFile(data)
          .then(function(filename) {
            unpackBundle(filename, reposPath);
          });
      } else if (typeof data == 'string') {
        unpackBundle(data, reposPath);
      } else {
        console.log('Failed to unpack bundle of type ' + typeof data);
      }
    }
  }
}


module.exports.makeBundleHandler = makeBundleHandler;
