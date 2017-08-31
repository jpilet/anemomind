
var mongoose = require('mongoose');
var ChartTile = require('../chart/charttile.model');

var dateLength = '2016-09-14T16:25:26'.length;

function pad(num, size) {
  var s = num+"";
  while (s.length < size) s = "0" + s;
  return s;
}


function formatTime(d) {
  // TODO: handle timezones
  //return d.toISOString();
  return [d.getFullYear(), pad(d.getMonth() + 1, 2), d.getDate()].join('-')
   + ' '
   + [pad(d.getHours(), 2), pad(d.getMinutes(), 2), pad(d.getSeconds(), 2)].join(':');
}

function getColumn (columns, title) {
  if (title in columns) {
    return columns[title];
  }
  return columns[title] = Object.keys(columns).length;
}

function getRow(table, timeSec) {
  var t = timeSec + '';
  if (!(t in table)) {
    table[t] = [ ];
  }
  return table[t];
}

function csvEscape(s) {
  return '"' + s.replace(/"/g, '""') + '"';
}

function sendCsv(res, columns, table) {
  var row = [ "Time" ];
  for (var c in columns) { row[1 + columns[c]] = csvEscape(c); }
  res.write(row.join(', ') + '\n');
  var numCols = row.length;

  var times = Object.keys(table);
  times.sort();

  for (var time_index = 0 ; time_index < times.length; ++ time_index) {
    var t = times[time_index];

    var rowDate = new Date(+t * 1000);

    row = [];
    row[0] = formatTime(rowDate);
    var tableRow = table[t];
    for (var i = 0; i < numCols; ++i) {
      row[i + 1] = (tableRow[i] !== undefined ? tableRow[i] : ''); 
    }
    res.write(row.join(', ') + '\n');
  }
}
    
exports.exportCsv = function(req, res, next) {
  var timeRange = req.params.timerange;
  var boat = req.params.boat;
  if (typeof(timeRange) != 'string' || timeRange.length != 2 * dateLength) {
    return res.status(400).send();
  }
  var start = new Date(timeRange.substr(0, dateLength) + 'Z');
  var end = new Date(timeRange.substr(dateLength, dateLength) + 'Z');
  if (isNaN(start.getTime()) || isNaN(end.getTime())) {
    return res.status(400).send();
  }

  // Fixed for now, might be configurable by the user later.
  var frequency = 1; // Hz

   // These values should match those in CharTiles.h
  var minZoom = 9;
  var maxZoom = 28;
  var samplesPerTile = 512;

  var zoom = Math.log2(samplesPerTile / frequency)

  var tileNo = function(date, rounding) {
    return Math[rounding](date.getTime() / 1000 / (1 << zoom));
  };

  // The following query fetches all data for a given tile:
  // db.charttiles.find({_id: {
  //   $gt: {
  //     "boat" : ObjectId("552b806a35ce7cb254dc9515"),
  //     "zoom" : 9,
  //     "tileno" : NumberLong(2799447),
  //     what: '',
  //     source: ''
  //   },
  //   $lt: {
  //     "boat" : ObjectId("552b806a35ce7cb254dc9515"),
  //     "zoom" : 9,
  //     "tileno" : NumberLong(2799447),
  //     what: 'zzz',
  //     source: 'zzz'
  //  } } }).forEach(function(t) { print(t._id.what +': ' + t._id.source); });

  var query = {
    _id: {
      $gt: {
        boat: mongoose.Types.ObjectId(boat),
        zoom: zoom,
        tileno: tileNo(start, 'floor'),
        what: '',
        source: ''
      },
      $lt: {
        boat: mongoose.Types.ObjectId(boat),
        zoom: zoom,
        tileno: tileNo(end, 'ceil'),
        what: 'zzz',
        source: 'zzz'
      }
    }
  };

  var columns = { };
  var table = { };

  var resultSent = false;

  // The query bypasses mongoose. It does not like having an object
  // as _id.
  ChartTile.collection.find(query).forEach(function(tile) {
    var columnTitle = tile._id.what + ' - ' + tile._id.source;
    var firstTime = new Date(1000 * tile._id.tileno * (1 << tile._id.zoom));

    // increment between samples in s
    var increment = (1 << tile._id.zoom) / samplesPerTile;
    var firstTimeSec = Math.floor(firstTime / 1000);

    var colno = getColumn(columns, columnTitle);
    for (var i = 0; i < samplesPerTile; ++i) {
      if (tile.count && tile.count[i] > 0) {
        var time = firstTimeSec + i * increment;
        var row = getRow(table, time);
        row[colno] = tile.mean[i];
      }
    }

    if (resultSent) {
      console.warn('ERROR! received DB data to build an reply that has been sent already!');
      console.warn(new Error().stack);
    }
  },
  function(err) {
    if (err) {
      res.status(500).send();
    } else {
      if (Object.keys(columns).length == 0) {
        res.status(404).send();
      } else {
        setTimeout(function() {
          res.contentType('text/csv');
          res.header("Content-Disposition", "attachment;filename=" + timeRange + ".csv");
          resultSent = true;
          sendCsv(res, columns, table);
          res.status(200).end();
        }, 1);
      }
    }
  });
}
