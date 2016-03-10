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

function bundleHandler(endpoint, packet) {
  
}

module.exports.bundleHandler = bundleHandler;
