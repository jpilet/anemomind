var files = require('../files.js');
var fs = require('fs');
var assert = require('assert');
var Q = require('q');
var endpoint = require('../endpoint.sqlite.js');
var sync2 = require('../sync2.js');
var common = require('../common.js');
var epschema = require('../endpoint-schema.js');
var should = require('should');

describe('files', function() {
  it('packfiles', function(done) {
    fs.writeFile('/tmp/filestest.txt', 'Some file data', function(err) {
      assert(!err);
      fs.readFile('/tmp/filestest.txt', function(err, refdata) {
        assert(!err);
        files.packFiles([{src:'/tmp/filestest.txt', dst:'mjao.txt'}]).then(function(values) {
          var value = values[0];
          assert(value.src.equals(refdata));
          assert.equal(value.dst, 'mjao.txt');
          var root = '/tmp/rulle/abc';
          files.unpackFiles(root, values).then(function(filenames) {
            assert.equal(filenames[0], '/tmp/rulle/abc/mjao.txt');
            fs.readFile('/tmp/rulle/abc/mjao.txt', function(err, unpackedData) {
              assert(!err);
              assert(unpackedData.equals(refdata));
              done();
            });
          });
        });
      });
    });
  });

  it('should transfer files', function(done) {
    Q.all([
      Q.nfcall(endpoint.tryMakeAndResetEndpoint, '/tmp/epa.db', 'a'),
      Q.nfcall(endpoint.tryMakeAndResetEndpoint, '/tmp/epb.db', 'b')
      ]).then(function(eps) {
        var a = eps[0];
        var b = eps[1];
        b.addPacketHandler(
          files.makePacketHandler('/tmp/boxdata', function(files) {
                                  console.log('**** FILES:');
                                  console.warn(files);
            files.should.eql(['/tmp/boxdata/boat.dat']);
            Q.all([
              Q.nfcall(fs.readFile, '/tmp/boxdata/boat.dat'),
              Q.nfcall(fs.readFile, '/tmp/boat.dat')
            ]).then(function(fdata) {
              assert(fdata[0].equals(fdata[1]));
              done();
            }).catch(done);
          }));
        epschema.makeVerbose(a);
        epschema.makeVerbose(b);
        
        var srcFilename = '/tmp/boat.dat';
        Q.nfcall(fs.writeFile, srcFilename, 'Interesting data for boat.dat')
          .then(function() {
            return files.sendFiles(a, 'b', [{src: srcFilename, dst: 'boat.dat'}]);
          })
          .then(function() {
            return Q.nfcall(sync2.synchronize, a, b);
          });
      });
  });
});
