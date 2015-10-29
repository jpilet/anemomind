var fs = require('fs');
var args = require('minimist')(process.argv.slice(2));
var Q = require('q');

var readJsonFile = function(file) {
  return Q.nfcall(fs.readFile, file)
    .then(function(data) {
          return JSON.parse(data);
    });
};

function traverseTree(tree, func) {
  func(tree);
  for (var i in tree.children) {
    traverseTree(tree.children[i], func);
  }
}

function nodesWithDescription(tree, description) {
  var nodes = [];
  traverseTree(tree, function(node) {
    if (node.description == description) {
      nodes.push(node);
    }
  });
  return nodes;
}

function linkForNode(boatid, node) {
  var stripZ = function(s) {
    return s.substring(0, s.length - 1);
  };

  return 'http://localhost:9000/map/' + boatid
    + '?c=' + boatid + stripZ(node.left) + stripZ(node.right);
}

function durationInSeconds(node) {
  return (new Date(node.right) - new Date(node.left)) / 1000;
}

function listRaces(path) {
  var boatId = path.match(/.*boat([a-zA-Z0-9]*)\/*/)[1];
  readJsonFile(path + '/processed/all_tree.js').then(function(tree) {
    var sessions = nodesWithDescription(tree, 'Sailing');
    for (var i in sessions) {
      var session = sessions[i];
      console.log('Session: ' + linkForNode(boatId, session));
      var races = nodesWithDescription(session, 'In race');
      for (var r in races) {
        console.log('  - ' + linkForNode(boatId, races[r])
         + ' (' + durationInSeconds(races[r]) / 60 + 'm)');
      }
    }
  })
  .catch(function(error) {
    console.warn(error);
    console.warn(error.stack);
  });
}

for (var i in args._) {
  listRaces(args._[i]);
}
