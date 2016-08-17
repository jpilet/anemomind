var assert = require('assert');
module.exports.prodassert = function(x) {
    if (process.env.DEBUG) { assert(x); }
}
