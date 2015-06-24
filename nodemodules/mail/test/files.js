var files = require('../files.js');
var fs = require('fs');
var assert = require('assert');
var Q = require('q');
var mail2 = require('../mail2.sqlite.js');

describe('File transfer code', function() {
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

  it('transferfiles', function(done) {
    Q.all([
      Q.nfcall(mail2.tryMakeAndResetEndPoint, '/tmp/epa.db', 'a'),
      Q.nfcall(mail2.tryMakeAndResetEndPoint, '/tmp/epb.db', 'b')
      ]).then(function(eps) {
        var a = eps[0];
        var b = eps[1];
        var srcFilename = '/tmp/boat.dat';
        Q.nfcall(fs.writeFile, srcFilename, 'Interesting data')
          .then(files.sendFiles(a, 'b', [{src: srcFilename, dst:'boat.dat'}]))
          .then(function() {
            done();
          });
      });
  });
});
