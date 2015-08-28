var fillTable = require('../../../RpcEndpoint.js').fillTable;
var table = {};

// TODO: This code is very similar to that in main.js.
// We should make sure that all functions available over
// the BT interface are also available over this interface.
fillTable(table);

function handleCall(fun, req, res) {
  try {
    fun(req.body, function(output) {
      res.status(200).json(output);
    });
  } catch (e) {
    res.status(500).json(
      {error: 'Something went wrong in rpc.controller.js on the anemobox. '+
        'Please make sure that none of the registered functions '+
        'throws any exception.'}
    );
  }
}

module.exports.index = function(req, res) {
  var fname = req.params.fname;
  if (table[fname]) {
    handleCall(table[fname], req, res);
  } else {
    res.status(404).json('No such function: ' + fname);
  }
}
