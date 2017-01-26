var testserver = require('./testserver.js');
var assert = require('assert');
var chai = require('chai');
var chaihttp = require('chai-http');

chai.use(chaihttp);

describe('app', function() {
  it('should export an app', function() {
    assert(testserver.app);
  });
});

describe('app', function() {
  it('should have some pine needle tea', function(done) {
    chai.request(testserver.app)
      .get('/')
      .end(function(err, res) {
        assert(res.status == 200);
        assert(res.text == "Pine needle tea");
        done();
      });
  });
});
