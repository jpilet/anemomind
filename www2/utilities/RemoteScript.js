var common = require('./RemoteScriptCommon.js');

common.init('production');

function sentCallback(err, reqCode) {
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
}

var databaseFilename = process.argv[2];
console.log('DB filename:     ' + databaseFilename);

/*

  Two different calling formats:

  (i)
  node RemoteScript.js dbFilename scriptFilename

     where scriptFilename should end with either .js or .sh

  or

  (ii)
  node RemoteScript.js dbFilename scriptType scriptData

     where scriptType is 'js' or 'sh', and
     scriptData is the script. If it is 'js', it
     should, when evaled, return a function that
     takes a single callback to which it passes its result.
*/

if (process.argv[4]) {
  var scriptType = process.argv[3];
  var scriptData = process.argv[4];
  console.log('Script type:     ' + scriptType);
  console.log('Script:          ' + scriptData);
  common.sendScriptToBox(databaseFilename, scriptType, scriptData, sentCallback);
} else {
  var scriptFilename = process.argv[3];
  console.log('Script filename: ' + scriptFilename);
  common.sendScriptFileToBox(databaseFilename, scriptFilename, sentCallback);
}
