var httputils = require('./httputils.js');

module.exports.tryMakeMailbox = httputils.makeEndPointConstructor(require('./mailbox-schema.js'));
