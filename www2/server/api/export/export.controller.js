
var mongoose = require('mongoose');
var ChartTile = require('../chart/charttile.model');
var ChartSource = require('../chart/chartsource.model');
var expandArrays = require('../chart/expand-array').expandArrays;

var esaFormat = require('./esaFormat');
var csvFormat = require('./csvFormat');

var ObjectId = mongoose.Types.ObjectId;

var dateLength = '2016-09-14T16:25:26'.length;


 // These values should match those in CharTiles.h
const minZoom = 9;
const maxZoom = 28;
const samplesPerTile = 512;

/*
function pad(num, size) {
  var s = num+"";
  while (s.length < size) s = "0" + s;
  return s;
}

function getColumn (columns, title) {
  if (title in columns) {
    return columns[title];
  }
  return columns[title] = Object.keys(columns).length;
}
*/

// Returns a row in the table as an array, 
// or allocates a new array if the row does not exist.
function getRow(table, timeSec) {
  var t = timeSec + '';
  if (!(t in table)) {
    table[t] = [ ];
  }
  return table[t];
}


var listChannelsWithSources = function(boat, zoom, firstTile, lastTile, cb) {
  ChartTiles.aggregate([
    {
      $match: {
        boat: ObjectId(boat),
        zoom: zoom,
        tileno: { $gte: firstTile, $lte: lastTile }
      },
    }, {
      $group: {
        _id: { what: "$what", source: "$source" },
        total: { $sum: 1 }
      }
    }, {
      $group: {
        _id: "$_id.what",
        sources: { $push: "$_id.source" }
      }
    }, {
      $project: {
        _id: false,
        what: "$_id",
        sources: true
      }
    }, {
      $sort: {what: 1}
    }
  ]).exec(function(err, data) {
    if (err) {
      cb(err);
      return;
    }
    ChartSource.findById(boat, (err, chartSources) => {
      if (err) {
        cb(err);
        return;
      }
      cb(null, data.map((x) => {
        return {
          what: x.what,
          sources: x.sources.map((source) => {
            return {
              source: source,
              prio: prioOfSource(x.what, source, chartSources)
            };
          }).sort((a, b) => b.prio - a.prio)
        };
      }));
    });
  });
};


var listColumns = function(boat, zoom, firstTile, lastTile, cb) {
  ChartTile.aggregate([
    {
      $match: {
        boat: ObjectId(boat),
        zoom: zoom,
        tileno: { $gte: firstTile, $lte: lastTile }
      },
    }, {
      $group: {
        _id: { what: "$what", source: "$source" },
      }
    }, {
      $sort: { _id: 1 }
    }
  ]).exec(function(err, res) {
    if (err) {
      cb(err);
    } else {
      cb(undefined, res.map(function(e) {
        return {
          source: e._id.source,
          type: e._id.what
        };
      }));
    }
  });
};

function sendWithColumns(
  outputFormat,
  chartSources,
  start, end, boat, zoom, firstTile, lastTile,
  columns, res, timeRange) {

  var query = {
    boat: mongoose.Types.ObjectId(boat),
    zoom: zoom,
    tileno: {
      "$gte": firstTile,
      "$lte": lastTile
    }
  };

  // Maps a time (in seconds) to arrays of mean values
  var table = { };

  // What type of data each column stores
  var columnType = { };

  var resultSent = false;

  var currentTile = firstTile;

  res.contentType(outputFormat.contentType);
  res.header("Content-Disposition", "attachment;filename=" + timeRange + outputFormat.fileExtension);
  outputFormat.sendHeader(res, columns, boat);

  var columnNames = columns.map((x) => outputFormat.columnString(x, chartSources));

  // The query bypasses mongoose.
  ChartTile.collection
    .find(query)
    // see https://docs.mongodb.com/manual/tutorial/sort-results-with-indexes/
    .sort({ boat:1, zoom:1, tileno: 1})
    .forEach(function(packedTile) {
      var tile = expandArrays(packedTile);

      var columnTitle = outputFormat.columnString({
        type: tile.what,
        source: tile.source
      }, chartSources);
      if (!columnTitle) {
        // column is ignored, skip it.
        return;
      }

      var firstTime = new Date(1000 * tile.tileno * (1 << tile.zoom));

      // Should we complete this chunk and move on?
      if (tile.tileno > currentTile) {
        // Send what we have.
        outputFormat.sendChunk(res, columns, table, columnType, columnNames);
        currentTile = tile.tileno;

        // Reset them, so that we can start over.
        table = { };
        columnType = { };
      }

      // increment between samples in s
      var increment = (1 << tile.zoom) / samplesPerTile;
      var firstTimeSec = Math.floor(firstTime / 1000);
      var startTimeSec = start.getTime() / 1000;
      var endTimeSec = end.getTime() / 1000;

      var colno = columnNames.indexOf(columnTitle);
      if (colno < 0) {
        return;
      }
      columnType[colno] = tile.what;

      for (var i = 0; i < samplesPerTile; ++i) {
        if (tile.count && tile.count[i] > 0) {
          var time = firstTimeSec + i * increment;
          if (time > startTimeSec && time < endTimeSec) {

            // Populate the table row
            var row = getRow(table, time);
            row[colno] = tile.mean[i];
          }
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
        setTimeout(function() {
          resultSent = true;
          outputFormat.sendChunk(res, columns, table, columnType, columnNames);
          res.status(200).end();
        }, 1);
      }
    });
}


function exportInFormat(format, req, res, next) {
  console.log('Export in format: ', format);
  var timeRange = req.params.timerange;
  var boat = req.params.boat + '';
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

  var zoom = Math.log2(samplesPerTile / frequency)

  var tileNo = function(date, rounding) {
    return Math[rounding](date.getTime() / 1000 / (1 << zoom));
  };

  var firstTile = tileNo(start, 'floor');
  var lastTile = tileNo(end, 'ceil');

  ChartSource.findById(boat, function(err, chartSources) {
    if (err) {
      console.warn(err);
      res.status(404).send();
    } else {

      console.log('Chart sources: %j', chartSources);

      listColumns(
        boat, zoom, firstTile, lastTile,
        function(err, columns) {
          if (err) {
            console.warn(err);
            res.status(500).send();
          } else if (columns.length == 0) {
<<<<<<< HEAD
=======
            console.warn('No columns! tiles: ', firstTile, ' -> ', lastTile);
>>>>>>> origin/jp-esa-polar-frontend
            res.status(404).send();
          } else {
            sendWithColumns(
              format,
              chartSources,
              start, end, boat, zoom, firstTile, lastTile,
              columns, res, timeRange);
          }
        });
    }
  });
};

exports.exportCsv = (req, res, next) => exportInFormat(csvFormat, req, res, next);
exports.exportEsa = (req, res, next) => exportInFormat(esaFormat, req, res, next);

