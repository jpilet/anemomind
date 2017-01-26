var testserver = require('./testserver.js');
var assert = require('assert');
var chai = require('chai');
var chaihttp = require('chai-http');

chai.use(chaihttp);

describe('app', function() {
  it('should export an app', function() {
    assert(testserver);
  });
});

describe('app', function() {
  it('should have some pine needle tea', function(done) {
    chai.request(testserver)
      .get('/')
      .end(function(err, res) {
        assert(res.status == 200);
        assert(res.text == "Pine needle tea");
        done();
      });
  });

  it('should access the endpoint', function(done) {
    chai.request(testserver)
      .get('/endpoint/info')
      .end(function(err, res) {
        assert(res.status == 200);
        assert(res.text == 'endpoint');
        done();
      });
  });
});
