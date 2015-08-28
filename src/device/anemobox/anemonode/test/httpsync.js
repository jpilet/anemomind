process.env.ANEMOBOX_CONFIG_PATH = '/tmp/anemoboxcfg';

var app = require('../components/http');
var lep = require('../components/LocalEndpoint.js');
var request = require('supertest');
var assert = require('assert');
var server = request(app);


describe('httpsync', function() {

  it('should successfully reset the endpoint', function(done) {
    lep.getName(function(lname) {
      server
        .get('/api/mailrpc/reset/' + lname)
        .set('Authorization', 'Bearer ')
        .expect(200)
        .end(function(err, res) {
	  if (err) return done(err);
	  done();
        });
    });
  });

/*  
  it('should fail to reset the endpoint', function(done) {
    server
      .get('/api/mailrpc/reset/unknownName')
      .expect(403)
      .end(function(err, res) {
	//if (err) return done(err);
	done();
      });
  });
  */
});
