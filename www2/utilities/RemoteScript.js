var common = require('./common.js');

var databaseFilename = process.argv[2];
var scriptType = process.argv[3]; // 'js' or 'sh'
var script = process.argv[4];
console.log('Database filename: ' + databaseFilename);
console.log('Script type: ' + scriptType);
console.log('Script: ' + script);
common.sendScriptToBox(databaseFilename, scriptType, script, function(err, reqCode) {
  if (err) {
    console.log('Failed to send script to box because');
    console.log(err);
  } else {
    console.log('Successfully posted script to box for remote execution.');
    console.log('Once a response has been returned, a document will be put');
    console.log('in the script collection of the MongoDB with reqCode = ');
    console.log(reqCode);
  }
});
