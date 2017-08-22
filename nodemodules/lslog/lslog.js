#! /usr/bin/env node

var glob = require("glob");
var fs = require("fs");


function show(filename) {
  var stats = fs.statSync(filename);
  var striped = filename.replace(/\.log$/, "");
  var secOrmillis = parseInt("0x" + striped);
  if (secOrmillis < 0x10000000000) {
    secOrmillis *= 1000;
  }
  var date = new Date(secOrmillis);
  var size = (stats.size / 1024).toFixed(2);
  console.log(filename + ' ' + date +' ' + size + 'kb');
}

glob("*.log", function (er, files) {
     files.sort();
     for (var i in files) {
       show(files[i]);
     }
})
