var httputils = require('./httputils.js');

module.exports.tryMakeEndPoint = httputils.makeEndPointConstructor(
  require('./endpoint-schema.js'));
