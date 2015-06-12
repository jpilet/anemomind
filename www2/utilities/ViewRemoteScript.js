var common = require('./RemoteScriptCommon.js');
common.init('production');
var BoxExec = require('../server/api/boxexec/boxexec.model.js');

function printIndentedIfNotNull(data, field) {
  if (data[field]) {
    console.log(field + ':');
    console.log('  ' + data[field]);
  }
}

function prettyPrintRequest(data) {
  console.log('REQUEST');
  printIndentedIfNotNull(data, 'boatId');
  printIndentedIfNotNull(data, 'boxId');
  printIndentedIfNotNull(data, 'type');
}

function prettyPrintResponse(data) {
  console.log('RESPONSE');
  printIndentedIfNotNull(data, 'err');
  printIndentedIfNotNull(data, 'stdout');
  printIndentedIfNotNull(data, 'stderr');
}

function prettyPrint(data) {
  console.log('Displaying boxexec ' + data._id + '\n');
  prettyPrintRequest(data);
  console.log('');
  if (data.timeReceived) {
    prettyPrintResponse(data);
  } else {
    console.log('Response has not arrived yet.');
  }
}

var id = process.argv[2];
BoxExec.findById(id, function(err, data) {
  if (err) {
    console.log('Failed to retrieve boxexec because');
    console.log(err);
  } else {
    prettyPrint(data);
  }
});
