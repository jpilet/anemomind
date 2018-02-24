var im = require('immutable');
var utils = require('./utils.js');
const dl = require('directory-list');
var sqlite3 = require('sqlite3');
var assert = require('assert');

function getStateDirs(statePath, cb) {
  dl.list(statePath, false, function(p) {
    var all = im.List(p.filter(utils.complement(utils.hiddenFilename)));
    var stateDirs = all.map(function(dir) {
      return im.Map({root: statePath, dir: dir});
    });
    cb(null, stateDirs);
  });
}

function listDeviceFiles(deviceData, cb) {
  var p = deviceData.get('root') + '/' + deviceData.get('dir');
  dl.list(p, false, function(files) {
    cb(null, deviceData.set('files', im.List(files)));
  });
}


function identifyDatabaseFile(deviceData) {
  var p = deviceData.get('files').filter(function(filename) {
    return utils.endsWith(filename, 'sqlite') || utils.endsWith(filename, 'sqlite.db');
  });
  return deviceData.set('dbfile', p.first());
}

function classifyDatabaseFile(deviceData) {
  var f = deviceData.get('dbfile');

  if (utils.endsWith(f, '.sqlite.db')) {
    return im.Map({type: 'box', dbformat: 'ours'});
  } else if (utils.endsWith(f, 'mailsqlite.db')) {
    return im.Map({type: 'server', dbformat: 'ours'});
  } else if (utils.endsWith(f, '.sqlite')) {
    return im.Map({type: 'iosapp', dbformat: 'CoreData'});
  }
  assert(false);
  return im.Map({});
}

function createDatabase(deviceData, cb) {
  var filename = im.List(['root', 'dir', 'dbfile']).map(
    function(k) {return deviceData.get(k)})
      .join('/');

  //var filename = deviceData.get('root') + 
  //    '/' + deviceData.get('dir') + '/' + deviceData.get('dbfile');
  var db = new sqlite3.Database(
    filename, sqlite3.OPEN_READONLY,
    function(err) {
      if (err) {
        cb(err);
      } else {
        cb(null, deviceData.set('db', db));
      }
    });
}

function closeDatabase(deviceData, cb) {
  deviceData.get('db').close(utils.forwardValue(deviceData, cb));
}

function getTableNames(deviceData, cb) {
  deviceData.get('db').all(
    'SELECT name FROM sqlite_master WHERE type="table"', function(err, result) {
      if (err) {
        cb(err);
      } else {
        cb(null, deviceData.set(
          'table-names', 
          im.Set(result.map(function(x) {return x.name;}))));
      }
    });
}

function tryGetBoatInfo(deviceData, cb) {
  if (deviceData.get('table-names').has('ZANMBOAT')) {
    deviceData.get('db').all('select *, ZANEMOBOX as anemobox from ZANMBOAT', function(err, result) {
      if (err) {
        cb(err);
      } else {
        cb(null, deviceData.set('boats', result));
      }
    });
  } else {
    cb(null, deviceData);
  }
}

function getTableSpecs(deviceData, cb) {
  var db = deviceData.get('db');
  var names = deviceData.get('table-names');
  return utils.asyncMap(
    function(x, cb) {
      db.all('PRAGMA table_info(' + x + ');', function(err, info) {
        if (err) {
          cb(err);
        } else {
          cb(null, im.List([x, info]));
        }
      });
    }, names, function(err, data) {
      if (err) {
        cb(err);
      } else {
        cb(null, deviceData.set('table-data', data));
      }
    });
}

function oursLowerBounds(deviceData, cb) {
  deviceData.get('db').all('select * from lowerBounds', function(err, result) {
    if (err) {
      cb(err);
    } else {
      cb(null, deviceData.set('lower-bounds', im.Set(result.map(im.Map))));
    }
  });
}

function lowerBoundFromCoreData(m) {
  return im.Map(m).update('lowerBound', function(lb) {
    return utils.padStart(16, '0', lb.toString(16));
  });
}

function coreDataLowerBounds(deviceData, cb) {
  deviceData.get('db').all(
    'select ZSRC as src, ZDST as dst, ZLOWERBOUND as lowerBound FROM ZANMLOWERBOUNDS', function(err, result) {
    if (err) {
      cb(err);
    } else {
      cb(null, 
         deviceData.set(
           'lower-bounds', 
           im.Set(result.map(lowerBoundFromCoreData))));
    }
  });
}

var getLowerBounds = utils.dispatchOnKey('dbformat', {
  ours: oursLowerBounds,
  CoreData: coreDataLowerBounds
});

function oursPacketInfo(deviceData, cb) {
  deviceData.get('db').all(
    'SELECT DISTINCT src, dst, count(*) as N FROM packets',
    function(err, result) {
      if (err) {
        cb(err);
      } else {
        //cb(null, deviceData);
        cb(null, deviceData.set('packet-info', im.Set(result.map(im.Map))));
      }
    });
}

function coreDataPacketInfo(deviceData, cb) {
  deviceData.get('db').all(
    'select ZSRC as src, ZDST as dst, count(*) as N from ZANMPACKET', 
    function(err, result) {
      if (err) {
        cb(err);
      } else {
        cb(null, deviceData.set('packet-info', im.Set(result.map(im.Map))));
      }
    });
}

function srcDstKey(src, dst) {
  //return im.Map({src: src, dst: dst});
  return im.Seq([src, dst]);
}


function lu(x, k) {
  //var y = x[k];
  //return y || x.get(k);
  return x.get(k);
}


function toSrcDstMap(arr) {
  return arr.reduce(function(m, x) {
    return m.set(srcDstKey(lu(x, 'src'), lu(x, 'dst')), x);
  }, im.Map({}));
}

function lowerBoundsAsMap(deviceData) {
  return deviceData.update('lower-bounds', toSrcDstMap);
}

function packetInfoAsMap(deviceData) {
  return deviceData.update('packet-info', toSrcDstMap);
}

/*function lowerBoundsAsMap(deviceData) {
  return deviceData.update('lower-bounds', toSrcDstMap);
}*/


var getPacketInfo = utils.dispatchOnKey('dbformat', {
  ours: oursPacketInfo,
  CoreData: coreDataPacketInfo
});

function srcDstSummary(deviceData) {
  function merger(a, b) {
    return a.merge(b);
  }
  var lbs = deviceData.get('lower-bounds');
  var packets = deviceData.get('packet-info');
  return lbs.mergeWith(merger, packets);
}

// A function that analyzes the state of
// a single device.
var analyzeDeviceState = utils.fwdcomp(
  listDeviceFiles,
  identifyDatabaseFile,
  utils.mergeDecorate(classifyDatabaseFile),
  createDatabase,
  getTableNames,
  getTableSpecs,
  getLowerBounds,
  getPacketInfo,
  tryGetBoatInfo,
  closeDatabase,
  lowerBoundsAsMap,
  packetInfoAsMap,
  utils.decorateAt('src-dst-summary', srcDstSummary),

  // So that it doesn't get so cluttered.
  utils.removeKeys(['table-data']),
  utils.removeKeys([
    'root', 'dbfile', 'files', 'db', 
    'table-names', 'lower-bounds', 'packet-info'
  ])
);

function analyzeEveryDeviceState(stateDirs, cb) {
  return utils.asyncMap(analyzeDeviceState, stateDirs, cb);
}

function initializeSummary(states) {
  return im.Map({
    devices: states,
    warnings: im.List(),
    errors: im.List()
  });
}

function computeFullSrcDstPairSet(summary) {
  return summary.set(
    'src-dst-pairs', 
    summary.get('devices').map(function(x) {
      return im.Set(x.get('src-dst-summary').keySeq());
    }).reduce(function(a, b) {return a.union(b);}));
}

function computeEndpointKeys(summary) {
  return summary.set(
    'endpoint-keys', 
    summary.get('src-dst-pairs').flatten().toSet());
}

function warnOnManyEps(type, msg) {
  return function (summary) {
    var eps = summary.get('endpoint-keys').filter(function(s) {
      return s.indexOf(type) == 0;
    });

    if (2 <= eps.count()) {
      return summary.update('warnings', function(w) {
        return w.push(im.Map({
          message: msg,
          data: eps
        }));
      });
    } else {
      return summary;
    }
  };
}

function display(summary) {
  console.log(JSON.stringify(summary, null, "  "));
}

// Analyze the state of several devices and produce a summary.
var fullAnalysis = utils.fwdcomp(
  getStateDirs,
  analyzeEveryDeviceState,
  initializeSummary,
  computeFullSrcDstPairSet,
  computeEndpointKeys,
  warnOnManyEps('box', 'There is more than one box among the endpoints'),
  warnOnManyEps('boat', 'There is more than one boat among the endpoints'),
  display
);

/*
  
  Tool to analyze the state of endpoints

  Usage:
  Put the database files (and other relevant files)
  in a subdirectory for every device. Currently, we
  only analyze sqlite databases. For instance, this 
  directory structure:

  device_states/
    the_server/
      boat9808098098098098.mailsqlite.db
    iosapp/
      asdfasdfsdf.sqlite
    box/
      box787980.sqlite.db

  and call this program providing the root path as argument, e.g.

     node index.js device_states

  It will traverse the directories, analyze the databases, and produce a 
  summary (currently not a very fancy one). The idea is that it should
  be able to point out anomalies that could cause synchronization problems.

  NOTE:
  The data for the iOS device can be obtained by creating
  a backup using the iMazing tool. That backup, in turn, can
  be unzipped and inside it, deep in the directory structure,
  is a file with the ending .sqlite containing the CoreData database.

*/
var args = process.argv.slice(2);
var statePath = args.length == 0? "/Users/jonas/benoit/dbstates" : args[0];
fullAnalysis(statePath, function(err) {
  if (err) {
    console.log(err);
  }
});

