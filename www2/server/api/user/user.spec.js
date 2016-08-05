'use strict';

var should = require('should');
var canonicalizeEmail = require('./user.controller').canonicalizeEmail;
var testUtils = require('../testUtils.spec');
var Q = require('q');

describe('canonicalizeEmail', function() {

  it('should trim and lowercase email addresses', function(done) {
     canonicalizeEmail(' Julien@Anemomind.com\n').should.equal('julien@anemomind.com');
     done();
  });

});
