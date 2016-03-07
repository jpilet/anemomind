var bundle = require('../bundle.js');
var assert = require('assert');
var Q = require('q');


function prepareTestSetup() {
  var x = Q.defer();
  x.resolve();
  return x.promise;  
}

describe('bundle', function() {
  it('Should prepare a setup where we can experiment', function(done) {
    prepareTestSetup()
      .then(done);
  });
});
