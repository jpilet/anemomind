var httputils = require('./httputils.js');

module.exports.tryMakeEndpoint = httputils.makeEndpointConstructor(
  require('./endpoint-schema.js'));
