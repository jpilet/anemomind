process.env.ANEMOBOX_CONFIG_PATH = '/tmp/anemoboxcfg';

var app = require('../components/http');
var lep = require('../components/LocalEndpoint.js');
var request = require('supertest');
var server = request(app);
var should = require('should');

var authorization = 'Bearer not in use for the anemobox, right?';

describe('httpsync', function() {
  it('should successfully reset the endpoint', function(done) {
    lep.getName(function(lname) {
      server
        .get('/api/mailrpc/reset/' + lname)
        .set('Authorization', authorization)
        .expect(200)
        .end(function(err, res) {
	  done(err);
        });
    });
  });
    
  it('should fail to reset the endpoint', function(done) {
    server
      .get('/api/mailrpc/reset/adsfadsf')
      .set('Authorization', 'Bearer ')
      .expect(500)
      .end(function(err, res) {
	done(err);
      });
  });

  it('should get the number of packets', function(done) {
    lep.getName(function(lname) {
      server
        .get('/api/mailrpc/getTotalPacketCount/' + lname)
        .set('Authorization', authorization)
        .expect(200)
        .end(function(err, res) {
	  if (err) return done(err);
	  JSON.parse(res.text).should.equal(0);
	  done();
        });
    });
  });
});
