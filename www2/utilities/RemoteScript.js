var common = require('./common.js');
var databaseFilename = process.argv[2];
var scriptFilename = process.argv[3];
console.log('Database filename: ' + databaseFilename);
console.log('Script filename:   ' + scriptFilename);
common.sendScriptFileToBox(databaseFilename, scriptFilename, function(err, reqCode) {
  if (err) {
    console.log('Failed to send script to box because');
    console.log(err);
  } else {
    console.log('Successfully posted script to box for remote execution.');
    console.log('Once a response has been returned, a document will be put');
    console.log('in the script collection of the MongoDB with reqCode = ');
    console.log(reqCode);
    console.log('You will also find the output in ');
    console.log('/tmp/scriptlogs/' + reqCode + '.txt');
  }
});
