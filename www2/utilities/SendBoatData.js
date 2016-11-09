var common = require('../server/api/boxexec/remoteOps');
common.init();

var boatId = process.argv[2];
var srcFilename = process.argv[3];
var dstFilename = process.argv[4];
common.sendBoatData(boatId, srcFilename, dstFilename, function(err) {
  if (err) {
    console.log('Failed to send boat data, because');
    console.log(err);
    process.exit(1);
  } else {
    console.log('Successfully posted boat data');
    process.exit(0);
  }
});
