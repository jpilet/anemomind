var files = require('../files.js');
var fs = require('fs');
var assert = require('assert');
var Q = require('q');
var mail2 = require('../mail2.sqlite.js');
var sync2 = require('../sync2.js');
var epschema = require('../endpoint-schema.js');


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

  it('promises', function(done) {
    function xfun(cb) {
      setTimeout(function() {cb(null, 119);}, 130);
    }

    function yfun(cb) {
      setTimeout(function() {cb(null, 8889);}, 140);
    }
    
    var X = Q.nfcall(xfun);
    var Y = Q.nfcall(yfun);

    X.then(Y).then(function(y) {
      console.log('y = ', y);
      //assert.equal(y, null);
      done();
    });
  });

  it('transferfiles', function(done) {
    Q.all([
      Q.nfcall(mail2.tryMakeAndResetEndPoint, '/tmp/epa.db', 'a'),
      Q.nfcall(mail2.tryMakeAndResetEndPoint, '/tmp/epb.db', 'b')
      ]).then(function(eps) {
        var a = eps[0];
        var b = eps[1];
        b.addPacketHandler(files.makePacketHandler('/tmp/boxdata', true));
        epschema.makeVerbose(a);
        epschema.makeVerbose(b);
        var srcFilename = '/tmp/boat.dat';
        Q.nfcall(fs.writeFile, srcFilename, 'Interesting data')
          .then(function() {
            return files.sendFiles(a, 'b', [{src: srcFilename, dst:'boat.dat'}]);
          })
          .then(function(v) {
            console.log('GOT THIS VALUE: ');
            console.log(v);
            a.getTotalPacketCount(function(err, n) {
              console.log('TOTAL COUNT: ' + n);
              done();
            });
          });
/*          .then(function(data) {
            a.getTotalPacketCount(function(err, n) {
              console.log(n);
              done();
            });
          })*/
 /*         .then(Q.nfcall(sync2.synchronize, a, b))
          .then(function() {
             done();
          });*/
      });
  });
});
