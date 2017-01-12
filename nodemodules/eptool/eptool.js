var ep = require('./endpoint.sqlite.js');
var epstate = require('./epstate.js');
var fs = require('fs');

function openExistingEndpoint(filename, cb) {
  fs.readFile(filename, function(err) {
    if (err) {
      cb(err);
    } else {
      ep.tryMakeEndpoint(filename, 'unknownname', cb);  
    }
  });
}

function report(err) {
  console.log("Got error " + err);
}

function viewFull(filename, full) {
  openExistingEndpoint(filename, function(err, ep) {
    if (err) {return report(err);}
    epstate.getAllEndpointData(ep, function(err, data) {
      if (err) {return report(err);}
      data = full? data : epstate.summarizeEndpointData(data);
      console.log("Endpoint contents:\n"
                  + JSON.stringify(data, null, 2));
    })
  });
}

function disp(filename) {
  viewFull(filename, true);
}

function summary(filename) {
  viewFull(filename, false);
}

function reset(filename) {
  openExistingEndpoint(filename, function(err, ep) {
    ep.reset(function(err) {
      if (err) {return report(err);}
      console.log("Successfully reset endpoint.");
    })
  });
}





////// Command line interface
args = process.argv.slice(2)
opkey = args[0]
filename = args[1]

var ops = {
  summary: summary,
  disp: disp,
  reset: reset
};

if (!(opkey in ops)) {
  console.log("No such operation: " + opkey);
  console.log("Available ops: ");
  for (var op in ops) {
    console.log('  * ' + op)
  }
  console.log('USAGE: node eptool.js [op] [filename]')
} else {
  ops[opkey](filename)
}
